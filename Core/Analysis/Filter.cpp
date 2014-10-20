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

// ITK
#include <itkMetaImageIO.h>
#include <itkChangeInformationImageFilter.h>

// Qt
#include <QDir>
#include <QMessageBox>
#include <QWidget>
#include <QDebug>
#include <QXmlStreamWriter>

// VTK
#include <vtkMath.h>

using namespace ESPINA;

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
    for(OutputSPtr output : m_outputs)
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
bool Filter::update()
{
  bool updated = true;

  if (numberOfOutputs() != 0 || createPreviousOutputs())
  {
    for(auto id : m_outputs.keys())
    {
      updated &= update(id);
    }
  }
  else
  {
    for(auto input : m_inputs)
    {
      input->output()->update(); //TODO: Move to input api?
    }
    execute();
  }

  return updated;
}

//----------------------------------------------------------------------------
bool Filter::update(Output::Id id)
{
  bool invalidateRegions = areEditedRegionsInvalidated();
  bool outputNeedsUpdate = needUpdate(id);

   if (invalidateRegions || outputNeedsUpdate)
   {
     // Invalidate previous edited regions
     if (invalidateRegions && validOutput(id))
     {
       m_outputs[id]->clearEditedRegions();
     }

     if (!fetchOutputData(id))
     {
       for(auto input : m_inputs)
       {
         input->output()->update();
       }

       execute(id);

       if (validOutput(id))
       {
         //m_outputs[id]->restoreEditedRegions(m_cacheDir, cacheOutputId(oId));
       }
     }
   }

   return true;
}

//----------------------------------------------------------------------------
Filter::Filter(InputSList inputs, Filter::Type type, SchedulerSPtr scheduler)
: Task                       {scheduler}
, m_analysis                 {nullptr}
, m_type                     {type}
, m_inputs                   {inputs}
, m_invalidateSortoredOutputs{false}
, m_fetchBehaviour           {nullptr}
{
  setName(m_type);
}

//----------------------------------------------------------------------------
bool Filter::fetchOutputData(Output::Id id)
{
  bool outputDataFetched = false;

  if (validStoredInformation() && m_fetchBehaviour)
  {
    QByteArray buffer = storage()->snapshot(outputFile());

    if (!buffer.isEmpty())
    {
      QXmlStreamReader xml(buffer);

      OutputSPtr output;
      DataSPtr   data;

      while (!xml.atEnd())
      {
        xml.readNextStartElement();
        if (xml.isStartElement())
        {
          if ("Output" == xml.name())
          {
            if (id == xml.attributes().value("id").toString().toInt())
            {
              // Outputs can be already created while checking if an output exists
              output = m_outputs.value(id, OutputSPtr{new Output(this, id)});

              auto spacing = xml.attributes().value("spacing");
              if (!spacing.isEmpty())
              {
                output->setSpacing(NmVector3(spacing.toString()));
              }
              m_outputs.insert(id, output);
            } else
            {
              output.reset();
            }
          }
          else if ("Data" == xml.name() && output)
          {
            m_fetchBehaviour->fetchOutputData(output, storage(), prefix(), xml.attributes());
          }
        }
      }
    }
    outputDataFetched = m_outputs.contains(id) && m_outputs[id]->isValid();
  }

  return outputDataFetched;
}

//----------------------------------------------------------------------------
void Filter::clearPreviousOutputs()
{
  m_outputs.clear();
  m_invalidateSortoredOutputs = true;
}

//----------------------------------------------------------------------------
bool Filter::validStoredInformation() const
{
  return !m_invalidateSortoredOutputs && storage() && !ignoreStorageContent();
}

//----------------------------------------------------------------------------
bool Filter::existOutput(Output::Id id) const
{
  if (m_outputs.isEmpty())
  {
    createPreviousOutputs();
  }

  return m_outputs.contains(id);
}

//----------------------------------------------------------------------------
bool Filter::createPreviousOutputs() const
{
  if (validStoredInformation())
  {
    QByteArray buffer = storage()->snapshot(outputFile());

    if (!buffer.isEmpty())
    {
      QXmlStreamReader xml(buffer);

      OutputSPtr output;

      while (!xml.atEnd())
      {
        xml.readNextStartElement();
        if (xml.isStartElement())
        {
          if ("Output" == xml.name())
          {
            int id = xml.attributes().value("id").toString().toInt();

            m_outputs.insert(id, OutputSPtr{new Output(const_cast<Filter *>(this), id)});
          }
        }
      }
    }
  }

  return !m_outputs.isEmpty();
}


//----------------------------------------------------------------------------
unsigned int Filter::numberOfOutputs() const
{
  return m_outputs.size();
}

//----------------------------------------------------------------------------
bool Filter::validOutput(Output::Id id) const
throw (Undefined_Output_Exception)
{
  return existOutput(id) && m_outputs[id]->isValid();
}

//----------------------------------------------------------------------------
OutputSPtr Filter::output(Output::Id id) const
throw (Undefined_Output_Exception)
{
  if (!existOutput(id)) throw Undefined_Output_Exception();

  return m_outputs[id];
}
