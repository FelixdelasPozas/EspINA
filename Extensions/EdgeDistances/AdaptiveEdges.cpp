/*
 *    <one line to give the program's name and a brief idea of what it does.>
 *    Copyright (C) 2012  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "AdaptiveEdges.h"

#include "EdgeDetector.h"
#include "EdgeDistance.h"
// #include "EdgeDistance.h"
#include <Core/Analysis/Channel.h>
#include <Core/Analysis/Extensions/SegmentationExtension.h>
#include <Core/Analysis/Segmentation.h>
#include <Core/Analysis/Data/VolumetricData.h>


#include <vtkAppendPolyData.h>
#include <vtkCellArray.h>
#include <vtkContourFilter.h>
#include <vtkDistancePolyDataFilter.h>
#include <vtkLine.h>
#include <vtkPointData.h>
#include <vtkPolyDataNormals.h>
#include <vtkPolyDataWriter.h>
#include <vtkReverseSense.h>
#include <vtkRuledSurfaceFilter.h>
#include <vtkSurfaceReconstructionFilter.h>
#include <vtkXMLPolyDataWriter.h>
#include <vtkGenericDataObjectWriter.h>
#include <vtkGenericDataObjectReader.h>

#include <QApplication>
#include <QDebug>
#include <QMessageBox>
#include <boost/iterator/iterator_concepts.hpp>

using namespace EspINA;

const ChannelExtension::Type AdaptiveEdges::TYPE = "AdaptiveEdges";

const QString AdaptiveEdges::EXTENSION_FILE = "AdaptiveEdges/AdaptiveEdges.ext";
const QString AdaptiveEdges::EDGES_FILE     = "AdaptiveEdges/ChannelEdges";
const QString AdaptiveEdges::FACES_FILE     = "AdaptiveEdges/ChannelFaces";


const QString AdaptiveEdges::EDGETYPE = "MarginType";

const std::string FILE_VERSION = AdaptiveEdges::TYPE.toStdString() + " 1.0\n";
const char SEP = ',';

//-----------------------------------------------------------------------------
AdaptiveEdges::AdaptiveEdges(bool          useAdaptiveEdges,
                             int           backgroundColor,
                             int           threshold,
                             SchedulerSPtr scheduler)
: m_backgroundColor(backgroundColor)
, m_computedVolume(0)
, m_threshold(threshold)
, m_useAdaptiveEdges(useAdaptiveEdges)
, m_edgeDetector(new EdgeDetector(this, scheduler))
{
}

//-----------------------------------------------------------------------------
AdaptiveEdges::~AdaptiveEdges()
{
  delete m_edgeDetector;
}

//-----------------------------------------------------------------------------
void AdaptiveEdges::computeAdaptiveEdges()
{
  //qDebug() << "Computing Adaptive Edges" << m_channel->data().toString() << AdaptiveEdgesID;
  m_edges = vtkSmartPointer<vtkPolyData>::New();
  m_useAdaptiveEdges = true;
  m_backgroundColor  = m_backgroundColor;
  m_threshold        = m_threshold;

  m_edgeDetector->setDescription(QObject::tr("Computing Edges %1").arg(m_channel->name()));

  m_edgeDetector->submit();
}

using VTKReader = vtkSmartPointer<vtkGenericDataObjectReader>;

//-----------------------------------------------------------------------------
void AdaptiveEdges::loadEdgesCache()
{
  m_mutex.lock();
  if (!m_edges.GetPointer() && !m_channel->isOutputModified())
  {
    QString edgesFile = EDGES_FILE + ".vtp";
    QFileInfo file(m_channel->storage()->absoluteFilePath(edgesFile));

    if (file.exists())
    {
      VTKReader reader = VTKReader::New();
      reader->SetFileName(file.absoluteFilePath().toUtf8());
      reader->SetReadAllFields(true);
      reader->Update();

      m_edges = vtkSmartPointer<vtkPolyData>(reader->GetPolyDataOutput());
    }
  }
  m_mutex.unlock();
}

//-----------------------------------------------------------------------------
void AdaptiveEdges::loadFacesCache()
{
  if (!m_faces[0].GetPointer() && !m_channel->isOutputModified())
  {
    for (int i = 0; i < 6; ++i)
    {
      QString edgesFile = QString(FACES_FILE + "-%1.vtp").arg(i);
      QFileInfo file(m_channel->storage()->absoluteFilePath(edgesFile));

      if (file.exists())
      {
        VTKReader reader = VTKReader::New();
        reader->SetFileName(file.absoluteFilePath().toUtf8());
        reader->SetReadAllFields(true);
        reader->Update();

        m_faces[i] = vtkSmartPointer<vtkPolyData>(reader->GetPolyDataOutput());
      }
    }
  }
}

using VTKWriter = vtkSmartPointer<vtkGenericDataObjectWriter>;

// //-----------------------------------------------------------------------------
// bool AdaptiveEdges::saveCache(Snapshot &snapshot)
// {
//   s_cache.purge();
// 
//   if (s_cache.isEmpty())
//     return false;
// 
//   std::ostringstream cache;
//   cache << FILE_VERSION;
// 
//   ChannelPtr channel;
//   foreach(channel, s_cache.keys())
//   {
//     ExtensionData &data = s_cache[channel].Data;
// 
//     QString channelId = channel->filter()->id();
// 
//     cache << channelId.toStdString();
//     cache << SEP << channel->outputId();
// 
//     cache << SEP << data.UseAdaptiveEdges;
//     cache << SEP << data.ComputedVolume;
//     cache << SEP << data.BackgroundColor;
//     cache << SEP << data.Threshold;
// 
//     cache << std::endl;
// 
//     VTKWriter polyWriter = VTKWriter::New();
// 
//     if (data.UseAdaptiveEdges)
//     {
//       loadEdgesCache(channel);
//       if (data.Edges)
//       {
//         channelId += QString("-%1").arg(channel->outputId());
//         polyWriter->SetInputData(data.Edges); 
//         polyWriter->SetFileTypeToBinary();
//         polyWriter->WriteToOutputStringOn();
//         polyWriter->Write();
// 
//         QByteArray  edges(polyWriter->GetOutputString(), polyWriter->GetOutputStringLength());
//         snapshot << SnapshotEntry(QString(EDGES_FILE + "%1.vtp").arg(channelId), edges);
//       }
// 
//       loadFacesCache(channel);
//       for(int i = 0; i < 6; i++)
//       {
//         if (data.Faces[i])
//         {
//           polyWriter->SetInputData(data.Faces[i]);
//           polyWriter->Update();
//           polyWriter->Write();
// 
//           QByteArray face(polyWriter->GetOutputString(), polyWriter->GetOutputStringLength());
//           snapshot << SnapshotEntry(QString(FACES_FILE + "%1_%2.vtp").arg(channelId).arg(i), face);
//         }
//       }
//     }
//   }
// 
//   snapshot << SnapshotEntry(EXTENSION_FILE, cache.str().c_str());
// 
// 
//   return true;
// }


using ComputedSegmentation = std::pair<unsigned int, unsigned long int>;

//-----------------------------------------------------------------------------
Snapshot AdaptiveEdges::snapshot() const
{
  return Snapshot();//TODO
}

//-----------------------------------------------------------------------------
void AdaptiveEdges::computeDistanceToEdge(SegmentationPtr segmentation)
{
  SegmentationExtensionSPtr extension = segmentation->extension(EdgeDistance::TYPE);
  Q_ASSERT(extension);
  EdgeDistance *distanceExt = edgeDistance(extension.get());

  auto volume = volumetricData(segmentation->output());
  Nm distance[6];
  Bounds segmentationBounds = segmentation->bounds();

  if (m_useAdaptiveEdges)
  {
    loadFacesCache();

    if (!m_faces[0])
      ComputeSurfaces();

    for(int face = 0; face < 6; face++)
    {
      vtkSmartPointer<vtkDistancePolyDataFilter> distanceFilter = vtkSmartPointer<vtkDistancePolyDataFilter>::New();
      distanceFilter->SignedDistanceOff();
      //TODO  distanceFilter->SetInputConnection( 0, meshRepresentation(segmentation->output())->mesh());
      distanceFilter->SetInputData(1, m_faces[face]);
      distanceFilter->Update();
      distance[face] = distanceFilter->GetOutput()->GetPointData()->GetScalars()->GetRange()[0];
    }
  }
  else
  {
    Bounds channelBounds = m_channel->bounds();

    for (int i = 0; i < 6; i+=2)
      distance[i] = segmentationBounds[i] - channelBounds[i];

    for (int i = 1; i < 6; i+=2)
      distance[i] = channelBounds[i] - segmentationBounds[i];
  }

  distanceExt->setDistances(distance);
}

//-----------------------------------------------------------------------------
vtkSmartPointer<vtkPolyData> AdaptiveEdges::channelEdges()
{
  loadEdgesCache();

  if (!m_edges.GetPointer()) computeAdaptiveEdges();

  // Ensure Margin Detector's finished
  m_mutex.lock();
  Q_ASSERT(m_edges.GetPointer());
  m_mutex.unlock();

  return m_edges;
}

//-----------------------------------------------------------------------------
Nm AdaptiveEdges::computedVolume()
{
  Nm volume = 0;

  loadEdgesCache();

  // Ensure Margin Detector's finished
  if (!m_edges.GetPointer()) 
  {
    computeAdaptiveEdges();
  }

  m_mutex.lock();
  volume = m_computedVolume;
  m_mutex.unlock();

  return volume;
}


//-----------------------------------------------------------------------------
void AdaptiveEdges::ComputeSurfaces()
{

  loadEdgesCache();

  if (!m_edges)
  {
    computeAdaptiveEdges();
  }

  m_mutex.lock();

  vtkPoints *borderPoints = m_edges->GetPoints();
  int numSlices = m_edges->GetNumberOfPoints()/4;

  for (int face = 0; face < 6; face++)
  {
    vtkSmartPointer<vtkPoints> facePoints = vtkSmartPointer<vtkPoints>::New();
    vtkSmartPointer<vtkCellArray> faceCells = vtkSmartPointer<vtkCellArray>::New();
    if (face < 4)
    {
      for (int i = 0; i < numSlices; i++)
      {
        switch(face)
        {
          case 0: // LEFT
            facePoints->InsertNextPoint(borderPoints->GetPoint(4*i));
            facePoints->InsertNextPoint(borderPoints->GetPoint((4*i)+1));
            break;
          case 1: // RIGHT
            facePoints->InsertNextPoint(borderPoints->GetPoint((4*i)+2));
            facePoints->InsertNextPoint(borderPoints->GetPoint((4*i)+3));
            break;
          case 2: // TOP
            facePoints->InsertNextPoint(borderPoints->GetPoint((4*i)+1));
            facePoints->InsertNextPoint(borderPoints->GetPoint((4*i)+2));
            break;
          case 3: // BOTTOM
            facePoints->InsertNextPoint(borderPoints->GetPoint((4*i)+3));
            facePoints->InsertNextPoint(borderPoints->GetPoint(4*i));
            break;
          default:
            Q_ASSERT(FALSE);
            break;
        }

        if (i == 0)
          continue;

        vtkIdType corners[4];
        corners[0] = (i*2)-2;
        corners[1] = (i*2)-1;
        corners[2] = (i*2)+1;
        corners[3] = 2*i;
        faceCells->InsertNextCell(4,corners);
      }
    }
    else
    {
      vtkIdType corners[4];
      switch(face)
      {
        case 4: // Front
          corners[0] = facePoints->InsertNextPoint(borderPoints->GetPoint(0));
          corners[1] = facePoints->InsertNextPoint(borderPoints->GetPoint(1));
          corners[2] = facePoints->InsertNextPoint(borderPoints->GetPoint(2));
          corners[3] = facePoints->InsertNextPoint(borderPoints->GetPoint(3));
          break;
        case 5: // Back
          corners[0] = facePoints->InsertNextPoint(borderPoints->GetPoint(borderPoints->GetNumberOfPoints()-4));
          corners[1] = facePoints->InsertNextPoint(borderPoints->GetPoint(borderPoints->GetNumberOfPoints()-3));
          corners[2] = facePoints->InsertNextPoint(borderPoints->GetPoint(borderPoints->GetNumberOfPoints()-2));
          corners[3] = facePoints->InsertNextPoint(borderPoints->GetPoint(borderPoints->GetNumberOfPoints()-1));
          break;
        default:
          Q_ASSERT(FALSE);
          break;
      }
      faceCells->InsertNextCell(4,corners);
    }

    vtkSmartPointer<vtkPolyData> poly = vtkSmartPointer<vtkPolyData>::New();
    poly->SetPoints(facePoints);
    poly->SetPolys(faceCells);

    m_faces[face] = poly;
  }
  m_mutex.unlock();
}

//-----------------------------------------------------------------------------
AdaptiveEdgesPtr EspINA::adaptiveEdges(ChannelExtensionPtr extension)
{
  return dynamic_cast<AdaptiveEdgesPtr>(extension);
}

//-----------------------------------------------------------------------------
AdaptiveEdgesSPtr EspINA::adaptiveEdges(ChannelPtr channel)
{
  auto extension = channel->extension(AdaptiveEdges::TYPE);

  return std::dynamic_pointer_cast<AdaptiveEdges>(extension);
}

//-----------------------------------------------------------------------------
AdaptiveEdgesSPtr EspINA::createAdaptiveEdgesIfNotAvailable(ChannelPtr channel)
{
  auto edgesExtension = adaptiveEdges(channel);

  if (!edgesExtension)
  {
    edgesExtension = AdaptiveEdgesSPtr(new AdaptiveEdges(true));
    channel->addExtension(edgesExtension);
  }
  Q_ASSERT(edgesExtension);

  return edgesExtension;
}
