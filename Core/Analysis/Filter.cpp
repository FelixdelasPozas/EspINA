/*

    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>

    This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

// ESPINA
#include "Filter.h"
#include <Core/Utils/BinaryMask.hxx>
#include <Core/Utils/TemporalStorage.h>
#include <Core/Utils/EspinaException.h>
#include <Core/IO/DataFactory/RawDataFactory.h>

// ITK
#include <itkMetaImageIO.h>
#include <itkChangeInformationImageFilter.h>

// Qt
#include <QDir>
#include <QMessageBox>
#include <QWidget>
#include <QXmlStreamWriter>

// VTK
#include <vtkMath.h>

using namespace ESPINA;
using namespace ESPINA::Core::Utils;

namespace ESPINA
{
  namespace OutputParser
  {
    bool isOutputSection(const QXmlStreamReader& xml)
    { return xml.name() == "Output"; }

    bool isDataSection(const QXmlStreamReader& xml)
    { return xml.name() == "Data"; }

    bool isEditedRegionSection(const QXmlStreamReader& xml)
    { return xml.name() == "EditedRegion"; }

    Output::Id parseOutputId(const QXmlStreamReader& xml)
    { return xml.attributes().value("id").toString().toInt(); }

    NmVector3 parseOutputSpacing(const QXmlStreamReader& xml)
    { return NmVector3(xml.attributes().value("spacing").toString()); }

    Bounds parseEditedRegionsBounds(const QXmlStreamReader& xml)
    { return Bounds(xml.attributes().value("bounds").toString()); }

    Data::Type parseDataType(const QXmlStreamReader& xml)
    { return xml.attributes().value("type").toString(); }
  }
}

using namespace ESPINA::OutputParser;

//----------------------------------------------------------------------------
Filter::~Filter()
{
}

//----------------------------------------------------------------------------
Snapshot Filter::snapshot() const
{
  Snapshot snapshot;

  if (!m_outputs.isEmpty())
  {
    QByteArray       buffer;
    QXmlStreamWriter xml(&buffer);

    xml.setAutoFormatting(true);
    xml.writeStartDocument();
    xml.writeStartElement("Filter");
    xml.writeAttribute("Type", m_type);

    for(auto output : m_outputs)
    {
      xml.writeStartElement("Output");
      xml.writeAttribute("id",      QString::number(output->id()));
      xml.writeAttribute("bounds",  output->bounds().toString());
      xml.writeAttribute("spacing", output->spacing().toString());
      snapshot << output->snapshot(storage(), xml, prefix());
      xml.writeEndElement();
    }

    xml.writeEndElement();
    xml.writeEndDocument();

    snapshot << SnapshotData(outputFile(), buffer);
  }

  snapshot << saveFilterSnapshot();

 return snapshot;
}

//----------------------------------------------------------------------------
void Filter::unload()
{
}

//----------------------------------------------------------------------------
void Filter::update()
{
  if (m_outputs.isEmpty() || needUpdate())
  {
    const bool invalidatePreviousEditedRegions = m_outputs.isEmpty() || ignoreStorageContent();

    for(auto input : m_inputs)
    {
      input->update();
    }

    execute();

    if (invalidatePreviousEditedRegions)
    {
      for (auto output : m_outputs)
      {
        output->clearEditedRegions();
      }
    }
    else
    {
       restoreEditedRegions();
    }
  }
}

//----------------------------------------------------------------------------
Filter::Filter(InputSList inputs, const Filter::Type &type, SchedulerSPtr scheduler)
: Task         {scheduler}
, m_analysis   {nullptr}
, m_type       {type}
, m_inputs     {inputs}
, m_dataFactory{new RawDataFactory()}
{
  setDescription(tr("Filter execution task: %1").arg(m_type));

  setName(m_type);
}

//----------------------------------------------------------------------------
void Filter::restoreEditedRegions()
{
  if (validStoredInformation())
  {
    QByteArray buffer;

    if(storage()->exists(outputFile()))
    {
      buffer = storage()->snapshot(outputFile());
    }

    if (!buffer.isEmpty())
    {
      QXmlStreamReader xml(buffer);

      OutputSPtr output;
      Data::Type dataType;
      BoundsList editedRegions;

      while (!xml.atEnd())
      {
        xml.readNextStartElement();
        if (xml.isStartElement())
        {
          if (isOutputSection(xml))
          {
            auto id = parseOutputId(xml);
            // Outputs have been already restored or creted by Filter::update()
            Q_ASSERT(m_outputs.contains(id));
            output = m_outputs.value(id);
            output->clearEditedRegions();
          }
          else if (isDataSection(xml) && output)
          {
            dataType = parseDataType(xml);
            editedRegions.clear();
          }
          else if (isEditedRegionSection(xml) && output)
          {
            editedRegions << parseEditedRegionsBounds(xml);
          }
        }
        else if (xml.isEndElement())
        {
          if (isDataSection(xml))
          {
            auto data = output->writeLockData<Data>(dataType);
            data->setEditedRegions(editedRegions);
            data->restoreEditedRegions(storage(), prefix(), QString::number(output->id()));
          }
        }
      }
    }
  }
}

//----------------------------------------------------------------------------
bool Filter::validStoredInformation() const
{
  return storage() && !ignoreStorageContent();
}

//----------------------------------------------------------------------------
bool Filter::existOutput(Output::Id id) const
{
  if (m_outputs.isEmpty())
  {
    restorePreviousOutputs();
  }

  return m_outputs.contains(id);
}

//----------------------------------------------------------------------------
bool Filter::restorePreviousOutputs() const
{
  if (validStoredInformation())
  {
    QByteArray buffer;

    if(storage()->exists(outputFile()))
    {
      buffer = storage()->snapshot(outputFile());
    }

    if (!buffer.isEmpty())
    {
      QXmlStreamReader xml(buffer);

      OutputSPtr output;
      DataSPtr   data;
      BoundsList editedRegions;

      while (!xml.atEnd())
      {
        xml.readNextStartElement();
        if (xml.isStartElement())
        {
          if (isOutputSection(xml))
          {
            auto id      = parseOutputId(xml);
            auto spacing = parseOutputSpacing(xml);

            output = std::make_shared<Output>(const_cast<Filter *>(this), id, spacing);
            m_outputs.insert(id, output);
          }
          else if (isDataSection(xml) && output)
          {
            // TODO: 2016-10-07 - @felix - Take into account data dependencies, requires data factory refactorization.
            data = m_dataFactory->createData(output, storage(), prefix(), xml.attributes());
            if (!data)
            {
              // TODO 2014-04-20: Create ReadOnlyData to preserve data information in further savings
              qWarning() << name() << ":" << uuid() << "Unable to create requested data type: prefix[" << prefix() << "]";
              for(auto attr: xml.attributes().toList())
              {
                qWarning() << "xml Attribute: " << attr.name().toString() << attr.value().toString();
              }
            }
            editedRegions.clear();
          }
          else if (isEditedRegionSection(xml) && output)
          {
            editedRegions << parseEditedRegionsBounds(xml);
          }
        }
        else if (xml.isEndElement())
        {
          if (isDataSection(xml))
          {
            if (data)
            {
              data->setEditedRegions(editedRegions);
            }
          }
        }
      }
    }
  }

  return !m_outputs.isEmpty();
}

//----------------------------------------------------------------------------
void Filter::changeSpacing(const NmVector3 &origin, const NmVector3 &spacing)
{
  for (auto output : outputs())
  {
    output->setSpacing(spacing);
  }
}

//----------------------------------------------------------------------------
unsigned int Filter::numberOfOutputs() const
{
  return m_outputs.size();
}

//----------------------------------------------------------------------------
bool Filter::validOutput(Output::Id id) const
{
  return existOutput(id) && m_outputs[id]->isValid();
}

//----------------------------------------------------------------------------
OutputSPtr Filter::output(Output::Id id) const
{
  if (!existOutput(id))
  {
    auto what = QObject::tr("Invalid output id, id: %1, filter: %2").arg(id).arg(type());
    auto details = QObject::tr("Filter::output(id) -> Invalid output id, id: %1, filter: %2").arg(id).arg(type());

    throw EspinaException(what, details);
  }

  return m_outputs[id];
}
