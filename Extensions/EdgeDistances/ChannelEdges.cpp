/*
 *
 *    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 *    This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
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

// ESPINA
#include "ChannelEdges.h"
#include "AdaptiveEdgesCreator.h"
#include "EdgesAnalyzer.h"
#include <Core/Analysis/Channel.h>
#include <Core/Analysis/Output.h>
#include <Core/Analysis/Segmentation.h>
#include <Core/Analysis/Data/VolumetricData.hxx>
#include <Core/Analysis/Data/MeshData.h>
#include <Core/Analysis/Data/VolumetricDataUtils.hxx>
#include <Core/Utils/vtkPolyDataUtils.h>

// VTK
#include <vtkAppendPolyData.h>
#include <vtkCellArray.h>
#include <vtkContourFilter.h>
#include <vtkDistancePolyDataFilter.h>
#include <vtkLine.h>
#include <vtkPoints.h>
#include <vtkPointData.h>
#include <vtkPolyDataNormals.h>
#include <vtkPolyDataWriter.h>
#include <vtkReverseSense.h>
#include <vtkRuledSurfaceFilter.h>
#include <vtkSurfaceReconstructionFilter.h>
#include <vtkXMLPolyDataWriter.h>
#include <vtkGenericDataObjectWriter.h>
#include <vtkGenericDataObjectReader.h>

// Qt
#include <QApplication>
#include <QDebug>
#include <QMessageBox>
#include <QThread>

using namespace ESPINA;

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
  if(!m_edgesAnalyzer->hasFinished())
  {
    m_edgesAnalyzer->abort();

    if (!m_edgesAnalyzer->thread()->wait(500))
      m_edgesAnalyzer->thread()->terminate();
  }

  m_edgesAnalyzer = nullptr;
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
void ChannelEdges::initializeEdges()
{
  loadEdgesCache();

  QWriteLocker lock(&m_edgesMutex);
  if (!m_edges.GetPointer())
  {
    computeAdaptiveEdges();
  }

  QReadLocker edgesLock(&m_edgesResultMutex);
}

//-----------------------------------------------------------------------------
void ChannelEdges::analyzeChannel()
{
  //qDebug() << "Launching Analyze Edges Task" << m_extendedItem->name();
  QWriteLocker lock(&m_edgesResultMutex);
  m_analysisResultMutex.lockForWrite();
  m_edgesAnalyzer->setDescription(QObject::tr("Analyzing Edges: %1").arg(m_extendedItem->name()));
  connect(m_edgesAnalyzer.get(), SIGNAL(finished()),
          this,                  SLOT(onChannelAnalyzed()));
  Task::submit(m_edgesAnalyzer);
}

//-----------------------------------------------------------------------------
void ChannelEdges::onChannelAnalyzed()
{
  //computeAdaptiveEdges();
}

//-----------------------------------------------------------------------------
void ChannelEdges::computeAdaptiveEdges()
{
  //qDebug() << "Launching Adaptive Edges Task" << m_extendedItem->name();
  m_edges = vtkSmartPointer<vtkPolyData>::New();

  QReadLocker lock(&m_analysisResultMutex);
  m_edgesResultMutex.lockForWrite();
  m_edgesCreator->setDescription(QObject::tr("Computing Edges %1").arg(m_extendedItem->name()));
  Task::submit(m_edgesCreator);
}

using VTKReader = vtkSmartPointer<vtkGenericDataObjectReader>;

//-----------------------------------------------------------------------------
void ChannelEdges::loadEdgesCache()
{
  QWriteLocker lock(&m_edgesMutex);
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
  QWriteLocker lock(&m_facesMutex);
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

  m_edgesMutex.lockForRead();
  if (m_edges)
  {
    auto name = snapshotName(EDGES_FILE);
    auto data = PolyDataUtils::savePolyDataToBuffer(m_edges);
    snapshot << SnapshotData(name, data);
  }
  m_edgesMutex.unlock();

  m_facesMutex.lockForRead();
  for (int i = 0; i < 6; ++i)
  {
    if (m_faces[i])
    {
      auto name = snapshotName(FACES_FILE.arg(i));
      auto data = PolyDataUtils::savePolyDataToBuffer(m_faces[i]);
      snapshot << SnapshotData(name, data);
    }
  }
  m_facesMutex.unlock();

  return snapshot;
}

//-----------------------------------------------------------------------------
void ChannelEdges::setUseDistanceToBounds(bool value)
{
  QWriteLocker lock(&m_analysisResultMutex);

  m_useDistanceToBounds = value;
  invalidate();
}

//-----------------------------------------------------------------------------
bool ChannelEdges::useDistanceToBounds() const
{
  QReadLocker lock(&m_analysisResultMutex);
  //qDebug() << "Accesing Distance Type" << m_useDistanceToBounds;

  return m_useDistanceToBounds;
}

//-----------------------------------------------------------------------------
itkVolumeType::RegionType ChannelEdges::sliceRegion(unsigned int slice) const
{
  itkVolumeType::RegionType region;

  auto origin = m_extendedItem->position();
  auto spacing = m_extendedItem->output()->spacing();

  if (useDistanceToBounds())
  {
    region = equivalentRegion<itkVolumeType>(origin, spacing, m_extendedItem->bounds());
    region.SetIndex(2, region.GetIndex(2) + slice);
    region.SetSize(2, 1);
  }
  else
  {
    const_cast<ChannelEdges *>(this)->initializeEdges();

    double LB[3], RT[3];

    m_edges->GetPoint(slice*4+0, LB);
    m_edges->GetPoint(slice*4+2, RT);

    Bounds bounds{LB[0], RT[0], RT[1], LB[1], LB[2], LB[2]};

    region = equivalentRegion<itkVolumeType>(origin, spacing, bounds);
  }

  return region;
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
void ChannelEdges::distanceToEdges(SegmentationPtr segmentation, Nm distances[6])
{
  loadFacesCache();

  m_facesMutex.lockForWrite();
  if (!m_faces[0])
  {
    computeSurfaces();
  }
  m_facesMutex.unlock();

  QReadLocker lock(&m_facesMutex);
  //qDebug() << "Computing distances";
  // BUG: fails when no mesh data is available
  vtkSmartPointer<vtkPolyData> segmentationMesh = vtkSmartPointer<vtkPolyData>::New();
  segmentationMesh->DeepCopy(meshData(segmentation->output())->mesh());
  for(int face = 0; face < 6; ++face)
  {
    //qDebug() << "Computing distance to face"<< face;
    vtkSmartPointer<vtkPolyData> faceMesh = vtkSmartPointer<vtkPolyData>::New();
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
  initializeEdges();

  //qDebug() << "Accesing Edges";
  vtkSmartPointer<vtkPolyData> result = vtkSmartPointer<vtkPolyData>::New();
  result->DeepCopy(m_edges);

  return result;
}

//-----------------------------------------------------------------------------
Nm ChannelEdges::computedVolume()
{
  initializeEdges();

  //qDebug() << "Accesing Edges";

  return m_computedVolume;
}

//-----------------------------------------------------------------------------
void ChannelEdges::setBackgroundColor(int value)
{
  QWriteLocker lock(&m_analysisResultMutex);

  if (m_backgroundColor != value)
  {
    m_backgroundColor = value;
    invalidate();
    // TODO update edges
  }
}

//-----------------------------------------------------------------------------
int ChannelEdges::backgroundColor() const
{
  QReadLocker lock(&m_analysisResultMutex);

  return m_backgroundColor;
}

//-----------------------------------------------------------------------------
void ChannelEdges::setThreshold(int value)
{
  QWriteLocker lock(&m_analysisResultMutex);
  if (m_threshold != value)
  {
    m_threshold = value;
    invalidate();
    // TODO update edges
  }
}

//-----------------------------------------------------------------------------
int ChannelEdges::threshold() const
{
  QReadLocker lock(&m_analysisResultMutex);

  return m_threshold;
}


//-----------------------------------------------------------------------------
void ChannelEdges::computeSurfaces()
{
  initializeEdges();

  vtkPoints *borderPoints = m_edges->GetPoints();
  int numSlices = m_edges->GetNumberOfPoints()/4;

  for (int face = 0; face < 6; face++)
  {
    vtkPoints    *facePoints = vtkPoints::New();
    vtkCellArray *faceCells  = vtkCellArray::New();
    if (face < 4)
    {
      for (int i = 0; i < numSlices; i++)
      {
        double p1[3], p2[3];
        switch(face)
        {
          case 0: // LEFT
            borderPoints->GetPoint((4*i)+0, p1);
            borderPoints->GetPoint((4*i)+1, p2);

            facePoints->InsertNextPoint(p1);
            facePoints->InsertNextPoint(p2);

            break;
          case 1: // RIGHT
            borderPoints->GetPoint((4*i)+2, p1);
            borderPoints->GetPoint((4*i)+3, p2);

            facePoints->InsertNextPoint(p1);
            facePoints->InsertNextPoint(p2);

            break;
          case 2: // TOP
            borderPoints->GetPoint((4*i)+1, p1);
            borderPoints->GetPoint((4*i)+2, p2);

            facePoints->InsertNextPoint(p1);
            facePoints->InsertNextPoint(p2);

            break;
          case 3: // BOTTOM
            borderPoints->GetPoint((4*i)+3, p1);
            borderPoints->GetPoint((4*i)+0, p2);

            facePoints->InsertNextPoint(p1);
            facePoints->InsertNextPoint(p2);

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
      double p[3];
      switch(face)
      {
        case 4: // Front
        {
          for (int i = 0; i < 4; ++i)
          {
            borderPoints->GetPoint(i, p);
            corners[i] = facePoints->InsertNextPoint(p);
          }
          break;
        }
        case 5: // Back
        {
          auto np = borderPoints->GetNumberOfPoints();
          for (int i = 0; i < 4; ++i)
          {
            borderPoints->GetPoint(np - (4-i), p);
            corners[i] = facePoints->InsertNextPoint(p);
          }
          break;
        }
        default:
          Q_ASSERT(false);
          break;
      }
      faceCells->InsertNextCell(4,corners);
    }

    vtkSmartPointer<vtkPolyData> poly = vtkSmartPointer<vtkPolyData>::New();
    poly->SetPoints(facePoints);
    poly->SetPolys(faceCells);

    facePoints->Delete();
    faceCells->Delete();

    m_faces[face] = poly;
 }
}

//-----------------------------------------------------------------------------
ChannelEdgesPtr ESPINA::channelEdgesExtension(ChannelExtensionPtr extension)
{
  return dynamic_cast<ChannelEdgesPtr>(extension);
}

//-----------------------------------------------------------------------------
ChannelEdgesSPtr ESPINA::channelEdgesExtension(ChannelPtr channel)
{
  auto extension = channel->extension(ChannelEdges::TYPE);

  return std::dynamic_pointer_cast<ChannelEdges>(extension);
}
