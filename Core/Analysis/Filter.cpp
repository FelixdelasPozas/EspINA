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

  if (numberOfOutputs() != 0 || restorePreviousOutputs())
  {
    for(auto id : m_outputs.keys())
    {
      updated &= update(id);
    }
  }
  else if (needUpdate())
  {
    for(auto input : m_inputs)
    {
      input->update();
    }

    execute();

    // If no previous output is restored then there is no edited
    // regions to be restored either
    for (auto output : m_outputs)
    {
      output->clearEditedRegions();
    }
  }

  return updated;
}

//----------------------------------------------------------------------------
bool Filter::update(Output::Id id)
{
   if (needUpdate(id))
   {
     OutputPtr prevOutput = nullptr;
     if (validOutput(id))
     {
       prevOutput = m_outputs[id].get();
     }

     // Invalidate previous edited regions
     bool invalidatePreviousEditedRegions = ignoreStorageContent();
     if (validOutput(id) && invalidatePreviousEditedRegions)
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

       Q_ASSERT(existOutput(id));
       // Existing output objects must remain between filter executions
       Q_ASSERT(!prevOutput || prevOutput == m_outputs[id].get());

       if (invalidatePreviousEditedRegions)
       {
         m_outputs[id]->clearEditedRegions();
       }
       else
       {
         restoreEditedRegions(id);
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
//, m_invalidateSortoredOutputs{false}
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

      qDebug() << buffer;

      OutputSPtr output;
      DataSPtr   data;
      BoundsList editedRegions;

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
             data = m_fetchBehaviour->fetchOutputData(output, storage(), prefix(), xml.attributes());
             data->clearEditedRegions();
             editedRegions.clear();
          }
          else if ("EditedRegion" == xml.name() && output)
          {
            editedRegions <<  Bounds(xml.attributes().value("bounds").toString());
            data->setEditedRegions(editedRegions);
          }
        }
      }
    }
    outputDataFetched = m_outputs.contains(id) && m_outputs[id]->isValid();
  }

  return outputDataFetched;
}

//----------------------------------------------------------------------------
void Filter::restoreEditedRegions(Output::Id id)
{
  if (validStoredInformation())
  {
    QByteArray buffer = storage()->snapshot(outputFile());

    //qDebug() << buffer;

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
          if ("Output" == xml.name())
          {
            if (id == xml.attributes().value("id").toString().toInt())
            {
              // Outputs can be already created while checking if an output exists
              Q_ASSERT(m_outputs.contains(id));
              output = m_outputs.value(id, OutputSPtr{});
              output->clearEditedRegions();
            } else
            {
              output.reset();
            }
          }
          else if ("Data" == xml.name() && output)
          {
            if (data)
            {
              data->restoreEditedRegions(storage(), prefix(), QString::number(output->id()));
            }
            data = output->data(xml.attributes().value("type").toString());
            if (data)
            {
              data->clearEditedRegions();
            }
          }
          else if ("EditedRegion" == xml.name() && output)
          {
            if (data)
            {
              editedRegions <<  Bounds(xml.attributes().value("bounds").toString());
              data->setEditedRegions(editedRegions);
            }
          }
        }
      }
      if (data)
      {
        data->restoreEditedRegions(storage(), prefix(), QString::number(output->id()));
      }
    }
  }
}

// //----------------------------------------------------------------------------
// void Filter::clearPreviousOutputs()
// {
//   m_outputs.clear();
//   m_invalidateSortoredOutputs = true;
// }

//----------------------------------------------------------------------------
bool Filter::validStoredInformation() const
{
  return /*!m_invalidateSortoredOutputs &&*/ storage() && !ignoreStorageContent();
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
