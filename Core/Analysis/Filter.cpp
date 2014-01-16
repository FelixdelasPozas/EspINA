/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  Jorge Peña Pastor <jpena@cesvima.upm.es>

    This program is free software: you can redistribute it and/or modify
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

// EspINA
#include "Filter.h"
#include <Core/Utils/BinaryMask.h>
#include <Core/Utils/TemporalStorage.h>
#include "Data/Volumetric/SparseVolume.h"
#include "Data/Mesh/RawMesh.h"

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

using namespace EspINA;

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

    for(OutputSPtr output : m_outputs)
    {
      xml.writeStartElement("Output");
      xml.writeAttribute("id",      QString::number(output->id()));
      xml.writeAttribute("bounds",  output->bounds().toString());
      xml.writeAttribute("spacing", output->spacing().toString());
      snapshot << output->snapshot(storage(), xml, prefix());
    }

    xml.writeEndDocument();

    // Prevent saving channel read only outputs
    if (!snapshot.isEmpty())
    {
      snapshot << SnapshotData(outputFile(), buffer);
    }
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
  if (numberOfOutputs() == 0)
  {
    for(auto input : m_inputs)
    {
      input->update();
    }

    execute();
  }
  else
  {
    for(int i = 0; i < m_outputs.size(); ++i)
    {
      update(i);
    }
  }
}

//----------------------------------------------------------------------------
bool Filter::update(Output::Id id)
{
  bool invalidateRegions = invalidateEditedRegions();
  bool outputNeedsUpdate = needUpdate(id);

   if (invalidateRegions || outputNeedsUpdate)
   {
     // Invalidate previous edited regions
     if (invalidateRegions && id < static_cast<unsigned int>(m_outputs.size()))
     {
       m_outputs[id]->clearEditedRegions();
     }

     if (!fetchOutputData(id))
     {
       for(OutputSPtr input : m_inputs)
       {
         input->update();
       }

       execute(id);

       if (id < static_cast<unsigned int>(m_outputs.size()))
       {
         //m_outputs[id]->restoreEditedRegions(m_cacheDir, cacheOutputId(oId));
       }
     }
   }
}


//----------------------------------------------------------------------------
Filter::Filter(OutputSList inputs, Filter::Type type, SchedulerSPtr scheduler)
: Task(scheduler)
, m_type(type)
, m_inputs(inputs)
{
  setName(m_type);
}

//----------------------------------------------------------------------------
bool Filter::fetchOutputData(Output::Id id)
{
  if (!ignoreStorageContent() && storage())
  {
    QByteArray buffer = storage()->snapshot(outputFile());

    if (!buffer.isEmpty())
    {
      QXmlStreamReader xml(buffer);

      OutputSPtr output;
      DataSPtr   data;

      m_outputs.clear();

      while (!xml.atEnd())
      {
        xml.readNextStartElement();
        if (xml.isStartElement())
        {
          if ("Output" == xml.name())
          {
            int id = xml.attributes().value("id").toString().toInt();
            output = OutputSPtr { new Output(this, id) };

            auto spacing = xml.attributes().value("spacing");
            if (!spacing.isEmpty())
            {
              output->setSpacing(NmVector3(spacing.toString()));
            }
          }
          else
            if ("Data" == xml.name())
            {
              if ("VolumetricData" == xml.attributes().value("type"))
              {
                data = DataSPtr { new SparseVolume<itkVolumeType>() };
              }
              else
                if ("MeshData" == xml.attributes().value("type"))
                {
                  //data = DataSPtr { new RawMesh() };
                }
            }
        }
        else
          if (xml.isEndElement())
          {
            if ("Output" == xml.name())
            {
              output->markToSave(true);
              m_outputs << output;
            }
            else
              if ("Data" == xml.name())
              {
                data->setOutput(output.get());
                if (data->fetchData(storage(), prefix()))
                {
                  output->setData(data);
                }
              }
          }
      }
    }
  }

  bool fetched = !m_outputs.isEmpty();

  for(auto output : m_outputs)
  {
    fetched &= output->isValid();
  }

  return fetched;
}


//----------------------------------------------------------------------------
unsigned int Filter::numberOfOutputs() const
{
  return m_outputs.size();
}

//----------------------------------------------------------------------------
bool Filter::validOutput(Output::Id id)
throw (Undefined_Output_Exception)
{
  return id < static_cast<unsigned int>(m_outputs.size()) && m_outputs[id]->isValid();
}

//----------------------------------------------------------------------------
OutputSPtr Filter::output(Output::Id id) const
throw (Undefined_Output_Exception)
{
  update(id);

  if (id >= static_cast<unsigned int>(m_outputs.size())) throw Undefined_Output_Exception();

  return m_outputs[id];
}


//     static const ModelItem::ArgumentId ID;
//     static const ModelItem::ArgumentId INPUTS;
//     static const ModelItem::ArgumentId EDIT;
// 
// static const QString CREATELINK;
//     
// const QString EspINA::Filter::CREATELINK = "CreateSegmentation";
// 
// typedef ModelItem::ArgumentId ArgumentId;
// 
// const ArgumentId Filter::ID     = "ID";
// const ArgumentId Filter::INPUTS = "Inputs";
// const ArgumentId Filter::EDIT   = "Edit"; // Backwards compatibility
// 
// typedef itk::ChangeInformationImageFilter<itkVolumeType> ChangeImageInformationFilter;
// 
// //----------------------------------------------------------------------------
// Filter::~Filter()
// {
// //   qDebug() << "Destroying Filter";
// }
// 
// //----------------------------------------------------------------------------
// void Filter::setCacheDir(QDir dir)
// {
//   m_cacheDir = dir;
// }
// 
// //----------------------------------------------------------------------------
// Filter::Filter(Filter::NamedInputs  namedInputs,
//                ModelItem::Arguments args,
//                FilterType           type)
// : m_namedInputs(namedInputs)
// , m_args(args)
// , m_type(type)
// , m_cacheId(-1)
// , m_traceable(false)
// , m_executed(false)
// {
//   if (m_args.contains(ID)) {
//     m_cacheId = m_args[ID].toInt();
//   } else {
//     m_args[ID] = "-1";
//   }
// }
// 
// //----------------------------------------------------------------------------
// QVariant Filter::data(int role) const
// {
//   if (Qt::DisplayRole == role)
//     return m_type;
//   else
//     return QVariant();
// }
// 
// //----------------------------------------------------------------------------
// QString Filter::serialize() const
// {
//   // NOTE: EDIT arg is being deprecated
//   Arguments copy = m_args;
//   copy.remove(EDIT);
// 
//   return copy.serialize();
// }
// 
// //----------------------------------------------------------------------------
// bool SegmentationFilter::needUpdate() const
// {
//   bool update = true;
// 
//   if (!m_outputs.isEmpty())
//   {
//     update = false;
//     foreach(SegmentationOutputSPtr filterOutput, m_outputs)
//     {
//       update = update || !filterOutput->isValid();
//     }
//   }
// 
//   return update;
// }
// 
// //----------------------------------------------------------------------------
// itkVolumeType::Pointer Filter::readVolumeFromCache(const QString &file)
// {
//   itkVolumeType::Pointer volume;
// 
//   if (m_cacheDir.exists(file))
//   {
//     itk::MetaImageIO::Pointer io = itk::MetaImageIO::New();
//     EspinaVolumeReader::Pointer reader = EspinaVolumeReader::New();
// 
//     QByteArray tmpFile = m_cacheDir.absoluteFilePath(file).toUtf8();
//     io->SetFileName(tmpFile);
//     reader->SetImageIO(io);
//     reader->SetFileName(tmpFile.data());
//     reader->Update();
// 
//     itkVolumeType::Pointer imagePtr = reader->GetOutput();
//     itkVolumeType::PointType origin = imagePtr->GetOrigin();
// 
//     // all itk volumes must have their origin at (0,0,0) to avoid
//     // origin discrepancies with their vtk counterparts.
//     if ((origin[0] != 0) || (origin[1] != 0) || (origin[2] != 0))
//     {
//       itkVolumeType::RegionType region = imagePtr->GetLargestPossibleRegion();
//       itkVolumeType::RegionType::IndexType index = region.GetIndex();
//       Q_ASSERT((index[0] == 0) && (index[1] == 0) && (index[2] == 0));
//       itkVolumeType::SpacingType spacing = imagePtr->GetSpacing();
// 
//       itkVolumeType::RegionType::IndexType newIndex;
//       newIndex[0] = vtkMath::Round(origin[0] / spacing[0]);
//       newIndex[1] = vtkMath::Round(origin[1] / spacing[1]);
//       newIndex[2] = vtkMath::Round(origin[2] / spacing[2]);
//       region.SetIndex(newIndex);
//       imagePtr->SetLargestPossibleRegion(region);
// 
//       itkVolumeType::PointType newOrigin;
//       newOrigin[0] = newOrigin[1] = newOrigin[2] = 0;
// 
//       ChangeImageInformationFilter::Pointer changer = ChangeImageInformationFilter::New();
//       changer->SetInput(imagePtr);
//       changer->ChangeOriginOn();
//       changer->ChangeRegionOff();
//       changer->SetOutputOrigin(newOrigin);
//       changer->Update();
// 
//       volume = changer->GetOutput();
//     }
//     else
//       volume = reader->GetOutput();
// 
//     volume->DisconnectPipeline();
//   }
// 
//   return volume;
// }
// //----------------------------------------------------------------------------
// bool SegmentationFilter::dumpSnapshot(Snapshot &snapshot)
// {
//   bool result = false;
// 
//   foreach(SegmentationOutputSPtr filterOutput, m_outputs)
//   {
//     FilterOutputId oId = filterOutput->id();
// 
//     // Backwards compatibility with seg files version 1.0
//     if (m_model->isTraceable() && m_args.contains(EDIT))
//     {
//       qWarning("Deprecated Format");
//       update(oId);
// 
//       QStringList outputIds = m_args[EDIT].split(",");
//       if (outputIds.contains(QString::number(oId)))
//       {
//         SegmentationVolumeSPtr editedVolume = segmentationVolume(filterOutput);
//         EspinaRegion region = editedVolume->espinaRegion();
//         editedVolume->addEditedRegion(region);
//       }
//     }
// 
//     bool saveEditedRegions = m_model->isTraceable() && filterOutput->isEdited();
// 
//     if (filterOutput->isCached() || saveEditedRegions)
//       result |= filterOutput->dumpSnapshot(cacheOutputId(oId), snapshot, saveEditedRegions);
// 
//     filterOutput->setCached(false);
//   }
// 
//   return result;
// }
// 
// //----------------------------------------------------------------------------
// bool ChannelFilter::needUpdate() const
// {
//   bool update = true;
// 
//   if (!m_outputs.isEmpty())
//   {
//     update = false;
//     foreach(ChannelOutputSPtr filterOutput, m_outputs)
//     {
//       update = update || !filterOutput->isValid();
//     }
//   }
// 
//   return update;
// }
// //----------------------------------------------------------------------------
// void SegmentationFilter::setCacheDir(QDir dir)
// {
//   Filter::setCacheDir(dir);
// 
//   //WARNING: We need to create all output, even if they are invalid (NULL volume pointer)
//   // Load cached outputs
//   Q_ASSERT(m_outputs.isEmpty());
//   // Compatibility: verion 3 seg files had only volume outputs shaved
//   // in the base cache dir. Thus, in case we find some files there, we must
//   // assign them to filter's volume outputs
//   bool v3SegFile = false;
// 
//   QStringList namedFilters;
//   namedFilters <<  QString("%1_*.mhd").arg(m_cacheId);
//   foreach(QString cachedFile, m_cacheDir.entryList(namedFilters))
//   {
//     v3SegFile = true;
// 
//     QStringList ids = cachedFile.section(".",0,0).split("_");
//     FilterOutputId oId = ids[1].toInt();
// 
//     if (!validOutput(oId))
//     {
//       createOutput(oId);
//       createRepresentationProxy(oId, SegmentationVolume::TYPE);
//       createRepresentationProxy(oId, MeshRepresentation::TYPE);
//     }
// 
//     SegmentationOutputSPtr editedOutput = m_outputs[oId];
//     QString outputName = cacheOutputId(oId);
// 
//     if (editedOutput->editedRegions().isEmpty() && m_cacheDir.exists(outputName + ".trc"))
//     {
//       QFile file(m_cacheDir.absoluteFilePath(outputName + ".trc"));
//       if (file.open(QIODevice::ReadOnly))
//       {
//         SegmentationVolumeSPtr volume = segmentationVolume(editedOutput);
//         while (!file.atEnd())
//         {
//           QByteArray line = file.readLine();
//           QStringList values = QString(line).split(" ");
//           Nm bounds[6];
//           for (int i = 0; i < 6; ++i)
//             bounds[i] = values[i].toDouble();
// 
//           volume->addEditedRegion(EspinaRegion(bounds));
//         }
//       }
//     }
//   }
// 
//   if (!v3SegFile)
//   {
//     QDir outputsDir(m_cacheDir.filePath("Outputs"));
// 
//     QStringList namedFilters;
//     namedFilters <<  QString("%1_*.trc").arg(m_cacheId);
//     foreach(QString outputInfoFile, outputsDir.entryList(namedFilters))
//     {
//       QStringList ids = outputInfoFile.section(".",0,0).split("_");
//       FilterOutputId oId = ids[1].toInt();
// 
//       createOutput(oId);
// 
//       QFile file(outputsDir.absoluteFilePath(outputInfoFile));
//       if (file.open(QIODevice::ReadOnly))
//       {
//         bool header   = true;
//         int  regionId = 0;
//         while (!file.atEnd())
//         {
//           QString line = file.readLine();
//           line = line.trimmed();
//           if (header)
//           {
//             if (!line.isEmpty())
//             {
//               createRepresentationProxy(oId, line);
//             } else
//             {
//               header = false;
//             }
//           } else // Restore edited regions
//           {
//             QStringList values = line.split(" ");
//             Q_ASSERT(values.size() == 7);
// 
//             EspinaRegion region;
//             for (int i = 0; i < 6; ++i)
//               region[i] = values[i+1].toDouble();
// 
//             m_outputs[oId]->representation(values[0])->addEditedRegion(region, regionId++);
//           }
//         }
//       }
//     }
//   }
// }
// 
// //----------------------------------------------------------------------------
// void SegmentationFilter::createOutput(FilterOutputId id)
// {
//   if (!m_outputs.contains(id))
//     m_outputs[id] = SegmentationOutputSPtr(new SegmentationOutput(this, id));
//   else
//     qWarning() << "Filter: " << data().toString() << " has already created output" << id;
// }
// 
// //----------------------------------------------------------------------------
// void SegmentationFilter::addOutputRepresentation(FilterOutputId id, SegmentationRepresentationSPtr rep)
// {
//   SegmentationRepresentationSList representacions;
//   representacions << rep;
// 
//   addOutputRepresentations(id, representacions);
// }
// 
// //----------------------------------------------------------------------------
// void SegmentationFilter::addOutputRepresentations(FilterOutputId id, SegmentationRepresentationSList repList)
// {
//   if (!m_outputs.contains(id))
//     createOutput(id);
// 
//   SegmentationOutputSPtr currentOutput = m_outputs[id];
// 
//   foreach(SegmentationRepresentationSPtr representation, repList)
//   {
//     SegmentationRepresentationSPtr proxyRepresentation = currentOutput->representation(representation->type());
//     representation->setOutput(currentOutput.get());
// 
//     if (!proxyRepresentation)
//       proxyRepresentation = createRepresentationProxy(id, representation->type());
// 
//     if (!proxyRepresentation->setInternalData(representation))
//     {
//       qWarning() << "Filter: Couldn't copy internal data";
//       Q_ASSERT(false);
//     }
//   }
// 
//   currentOutput->clearGraphicalRepresentations();
// 
//   if (m_graphicalRepresentationFactory)
//     m_graphicalRepresentationFactory->createGraphicalRepresentations(currentOutput);
// }
// 
