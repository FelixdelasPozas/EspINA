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

#include "ChannelEdges.h"

#include "AdaptiveEdgesCreator.h"
#include "EdgesAnalyzer.h"
#include <Core/Analysis/Channel.h>
#include <Core/Analysis/Segmentation.h>
#include <Core/Analysis/Data/VolumetricData.h>
#include <Core/Analysis/Data/MeshData.h>
#include <Core/Utils/vtkPolyDataUtils.h>

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

const ChannelExtension::Type ChannelEdges::TYPE = "AdaptiveEdges";

const QString ChannelEdges::EDGES_FILE = "ChannelEdges.vtp";
const QString ChannelEdges::FACES_FILE = "ChannelFaces_%1.vtp";

const std::string FILE_VERSION = ChannelEdges::TYPE.toStdString() + " 2.0\n";
const char SEP = ',';

//-----------------------------------------------------------------------------
ChannelEdges::ChannelEdges(SchedulerSPtr                     scheduler,
                           const ChannelExtension::InfoCache &cache,
                           const State                       &state)
: ChannelExtension(cache)
, m_backgroundColor(-1)
, m_computedVolume(0)
, m_threshold(-1)
, m_edgesCreator (AdaptiveEdgesCreatorSPtr{new AdaptiveEdgesCreator(this, scheduler)})
, m_edgesAnalyzer(EdgesAnalyzerSPtr{new EdgesAnalyzer(this, scheduler)})
{
  if (!state.isEmpty())
  {
    //State: UseDistanceToBounds,BackgroundColor,Threshold
    auto values = state.split(",");
    m_useDistanceToBounds = values[0].toInt();
    m_backgroundColor     = values[1].toInt();
    m_threshold           = values[2].toInt();
  }
}

//-----------------------------------------------------------------------------
ChannelEdges::~ChannelEdges()
{
  m_edgesAnalyzer->abort();
  m_edgesAnalyzer->abort();
}

//-----------------------------------------------------------------------------
void ChannelEdges::onExtendedItemSet(Channel *item)
{
  if (m_threshold == -1)
  {
    analyzeChannel();
  }
}

//-----------------------------------------------------------------------------
void ChannelEdges::analyzeChannel()
{
  m_mutex.lock();
  //qDebug() << "Analyzing Channel" << m_extendedItem->name();
  m_edgesAnalyzer->setDescription(QObject::tr("Analyzing Edges: %1").arg(m_extendedItem->name()));
  connect(m_edgesAnalyzer.get(), SIGNAL(finished()),
          this,                  SLOT(onChannelAnalyzed()));
  Task::submit(m_edgesAnalyzer);
}

//-----------------------------------------------------------------------------
void ChannelEdges::onChannelAnalyzed()
{
  computeAdaptiveEdges();
}

//-----------------------------------------------------------------------------
void ChannelEdges::computeAdaptiveEdges()
{
  //qDebug() << "Computing Adaptive Edges" << m_extendedItem->name();
  m_edges = vtkSmartPointer<vtkPolyData>::New();

  m_mutex.lock();
  m_edgesCreator->setDescription(QObject::tr("Computing Edges %1").arg(m_extendedItem->name()));
  Task::submit(m_edgesCreator);
}

using VTKReader = vtkSmartPointer<vtkGenericDataObjectReader>;

//-----------------------------------------------------------------------------
void ChannelEdges::loadEdgesCache()
{
  QMutexLocker lock(&m_mutex);

  if (!m_edges.GetPointer() && !m_extendedItem->isOutputModified())
  {
    QString   snapshot  = snapshotName(EDGES_FILE);
    QFileInfo edgesFile = m_extendedItem->storage()->absoluteFilePath(snapshot);

    if (edgesFile.exists())
    {
      m_edges = PolyDataUtils::readPolyDataFromFile(edgesFile.absoluteFilePath());
    }
  }
}

//-----------------------------------------------------------------------------
void ChannelEdges::loadFacesCache()
{
  QMutexLocker lock(&m_mutex);

  if (!m_faces[0].GetPointer() && !m_extendedItem->isOutputModified())
  {
    for (int i = 0; i < 6; ++i)
    {
      QString   snapshot  = snapshotName(FACES_FILE.arg(i));
      QFileInfo facesFile = m_extendedItem->storage()->absoluteFilePath(snapshot);

      if (facesFile.exists())
      {
        m_faces[i] = PolyDataUtils::readPolyDataFromFile(facesFile.absoluteFilePath());
      }
    }
  }
}

using VTKWriter = vtkSmartPointer<vtkGenericDataObjectWriter>;

using ComputedSegmentation = std::pair<unsigned int, unsigned long int>;

//-----------------------------------------------------------------------------
State ChannelEdges::state() const
{
  return QString("%1,%2,%3").arg(m_useDistanceToBounds)
                            .arg(m_backgroundColor)
                            .arg(m_threshold);
}

//-----------------------------------------------------------------------------
Snapshot ChannelEdges::snapshot() const
{
  Snapshot snapshot;

  if (m_edges)
  {
    auto name = snapshotName(EDGES_FILE);
    auto data = PolyDataUtils::savePolyDataToBuffer(m_edges);
    snapshot << SnapshotData(name, data);
  }

  for (int i = 0; i < 6; ++i)
  {
    if (m_faces[i])
    {
      auto name = snapshotName(FACES_FILE.arg(i));
      auto data = PolyDataUtils::savePolyDataToBuffer(m_faces[i]);
      snapshot << SnapshotData(name, data);
    }
  }

  return snapshot;
}

//-----------------------------------------------------------------------------
void ChannelEdges::distanceToBounds(SegmentationPtr segmentation, Nm distances[6]) const
{
  Bounds channelBounds      = m_extendedItem->bounds();
  Bounds segmentationBounds = segmentation->bounds();

  for (int i = 0; i < 6; i+=2)
    distances[i] = segmentationBounds[i] - channelBounds[i];

  for (int i = 1; i < 6; i+=2)
    distances[i] = channelBounds[i] - segmentationBounds[i];
}

//-----------------------------------------------------------------------------
void ChannelEdges::distanceToEdges(SegmentationPtr segmentation, Nm distances[6]) const
{
  Bounds segmentationBounds = segmentation->bounds();

  loadFacesCache();

  if (!m_faces[0])
  {
    ComputeSurfaces();
  }

  for(int face = 0; face < 6; ++face)
  {
    // BUG: fails when no mesh data is available
    auto segmentationMesh = vtkSmartPointer<vtkPolyData>::New();
    auto faceMesh         = vtkSmartPointer<vtkPolyData>::New();

    segmentationMesh->DeepCopy(meshData(segmentation->output())->mesh());
    faceMesh->DeepCopy(m_faces[face]);

    vtkSmartPointer<vtkDistancePolyDataFilter> distanceFilter = vtkSmartPointer<vtkDistancePolyDataFilter>::New();
    distanceFilter->SignedDistanceOff();
    distanceFilter->SetInputData(0, segmentationMesh);
    distanceFilter->SetInputData(1, faceMesh);
    distanceFilter->Update();
    distances[face] = distanceFilter->GetOutput()->GetPointData()->GetScalars()->GetRange()[0];
  }
}

//-----------------------------------------------------------------------------
vtkSmartPointer<vtkPolyData> ChannelEdges::channelEdges()
{
  loadEdgesCache();

  if (!m_edges.GetPointer())
  {
    computeAdaptiveEdges();
  }

  // Ensure Margin Detector's finished
  m_mutex.lock();
  Q_ASSERT(m_edges.GetPointer());
  m_mutex.unlock();

  return m_edges;
}

//-----------------------------------------------------------------------------
Nm ChannelEdges::computedVolume()
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
void ChannelEdges::setBackgroundColor(int value)
{
  if (m_backgroundColor != value)
  {
    m_backgroundColor = value;
    // TODO update edges
  }
}

//-----------------------------------------------------------------------------
void ChannelEdges::setThreshold(int value)
{
  if (m_threshold != value)
  {
    m_threshold = value;
    // TODO update edges
  }
}


//-----------------------------------------------------------------------------
void ChannelEdges::ComputeSurfaces()
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
ChannelEdgesPtr EspINA::channelEdgesExtension(ChannelExtensionPtr extension)
{
  return dynamic_cast<ChannelEdgesPtr>(extension);
}

//-----------------------------------------------------------------------------
ChannelEdgesSPtr EspINA::channelEdgesExtension(ChannelPtr channel)
{
  auto extension = channel->extension(ChannelEdges::TYPE);

  return std::dynamic_pointer_cast<ChannelEdges>(extension);
}

// //-----------------------------------------------------------------------------
// ChannelEdgesSPtr EspINA::createAdaptiveEdgesIfNotAvailable(ChannelPtr channel)
// {
//   auto edgesExtension = channelEdgesExtension(channel);
//
//   if (!edgesExtension)
//   {
//     edgesExtension = ChannelEdgesSPtr(new ChannelEdges());
//     channel->addExtension(edgesExtension);
//   }
//   Q_ASSERT(edgesExtension);
//
//   return edgesExtension;
// }
