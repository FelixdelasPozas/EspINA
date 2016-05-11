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

using VTKReader = vtkSmartPointer<vtkGenericDataObjectReader>;
using VTKWriter = vtkSmartPointer<vtkGenericDataObjectWriter>;
using ComputedSegmentation = std::pair<unsigned int, unsigned long int>;

//-----------------------------------------------------------------------------
ChannelEdges::ChannelEdges(SchedulerSPtr                     scheduler,
                           const ChannelExtension::InfoCache &cache,
                           const State                       &state)
: ChannelExtension     {cache}
, m_hasAnalizedChannel {false}
, m_hasCreatedEdges    {false}
, m_useDistanceToBounds{true}
, m_backgroundColor    {-1}
, m_threshold          {-1}
, m_computedVolume     {0}
, m_invalidated        {false}
, m_edgesCreator       {std::make_shared<AdaptiveEdgesCreator>(this, scheduler)}
, m_edgesAnalyzer      {std::make_shared<EdgesAnalyzer>(this, scheduler)}
, m_edges              {nullptr}
, m_scheduler          {scheduler}
{
  for(int i = 0; i < 6; ++i)
  {
    m_faces[i] = nullptr;
  }

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
  invalidateResults();

  m_analisysWait.wakeAll();
  m_edgesTask.wakeAll();

  m_edgesAnalyzer = nullptr;
  m_edgesCreator = nullptr;
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
  if(!m_invalidated)
  {
    // NOTE: avoid loading old information if this extension has been invalidate on this session.
    loadEdgesData();
  };

  checkEdgesData();
}

//-----------------------------------------------------------------------------
void ChannelEdges::analyzeChannel()
{
  {
    QReadLocker lock(&m_dataMutex);

    if(m_edgesAnalyzer->isRunning() || m_edgesAnalyzer->hasFinished()) return;
  }

  QWriteLocker lock(&m_dataMutex);

  m_edgesAnalyzer->setDescription(QObject::tr("Analyzing Edges: %1").arg(m_extendedItem->name()));

  Task::submit(m_edgesAnalyzer);
}

//-----------------------------------------------------------------------------
void ChannelEdges::computeAdaptiveEdges()
{
  checkAnalysisData();

  {
    QReadLocker lock(&m_dataMutex);

    if(m_edgesCreator->isRunning() || m_edgesCreator->hasFinished()) return;
  }

  QWriteLocker lock(&m_dataMutex);

  m_edgesCreator->setDescription(QObject::tr("Computing Edges: %1").arg(m_extendedItem->name()));

  Task::submit(m_edgesCreator);
}

//-----------------------------------------------------------------------------
void ChannelEdges::loadEdgesData()
{
  QWriteLocker lock(&m_dataMutex);

  if (!m_edges.GetPointer() && !m_extendedItem->isOutputModified())
  {
    QString   snapshot  = snapshotName(EDGES_FILE);
    QFileInfo edgesFile = m_extendedItem->storage()->absoluteFilePath(snapshot);

    if (edgesFile.exists())
    {
      m_edges = PolyDataUtils::readPolyDataFromFile(edgesFile.absoluteFilePath());
    }
  }

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

  m_hasCreatedEdges = m_edges.GetPointer() &&
                      m_faces[0].GetPointer() &&
                      m_faces[1].GetPointer() &&
                      m_faces[2].GetPointer() &&
                      m_faces[3].GetPointer() &&
                      m_faces[4].GetPointer() &&
                      m_faces[5].GetPointer();
}

//-----------------------------------------------------------------------------
State ChannelEdges::state() const
{
  checkAnalysisData();

  return QString("%1,%2,%3").arg(m_useDistanceToBounds)
                            .arg(m_backgroundColor)
                            .arg(m_threshold);
}

//-----------------------------------------------------------------------------
Snapshot ChannelEdges::snapshot() const
{
  QReadLocker lock(&m_dataMutex);

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
  {
    distances[i] = segmentationBounds[i] - channelBounds[i];
  }

  for (int i = 1; i < 6; i+=2)
  {
    distances[i] = channelBounds[i] - segmentationBounds[i];
  }
}

//-----------------------------------------------------------------------------
void ChannelEdges::distanceToEdges(SegmentationPtr segmentation, Nm distances[6])
{
  initializeEdges();

  auto output = segmentation->output();

//   qDebug() << "Computing distances";
  auto segmentationPolyData = vtkSmartPointer<vtkPolyData>::New();
  if (hasMeshData(output))
  {
    segmentationPolyData->DeepCopy(readLockMesh(output)->mesh());
    for(int face = 0; face < 6; ++face)
    {
      //qDebug() << "Computing distance to face"<< face;
      auto faceMesh = vtkSmartPointer<vtkPolyData>::New();
      faceMesh->DeepCopy(m_faces[face]);

      auto distanceFilter = vtkSmartPointer<vtkDistancePolyDataFilter>::New();
      distanceFilter->SignedDistanceOff();
      distanceFilter->SetInputData(0, segmentationPolyData);
      distanceFilter->SetInputData(1, faceMesh);
      distanceFilter->Update();
      distances[face] = distanceFilter->GetOutput()->GetPointData()->GetScalars()->GetRange()[0];
    }
  }
//   else if (hasSkeletonData(output))
//   {
//     segmentationPolyData->DeepCopy(readLockSkeleton(output)->skeleton());
//   }
  else
  {
    qWarning() << tr("Unavailable mesh information");
    for (int i = 0; i < 6; ++i)
    {
      distances[i] = -1;
    }
  }
}

//-----------------------------------------------------------------------------
vtkSmartPointer<vtkPolyData> ChannelEdges::channelEdges()
{
  initializeEdges();

  auto result = vtkSmartPointer<vtkPolyData>::New();
  {
    QReadLocker lock(&m_dataMutex);
    result->DeepCopy(m_edges);
  }

  return result;
}

//-----------------------------------------------------------------------------
Nm ChannelEdges::computedVolume()
{
  initializeEdges();

  return m_computedVolume;
}

//-----------------------------------------------------------------------------
QString ChannelEdges::snapshotName(const QString& file) const
{
  auto channelName = m_extendedItem->name();

  return QString("%1/%2/%3_%4").arg(Path())
                               .arg(type())
                               .arg(channelName.remove(' ').replace('.','_'))
                               .arg(file);
}

//-----------------------------------------------------------------------------
void ChannelEdges::invalidate()
{
  invalidateResults();
  m_invalidated = true;

  emit invalidated();
}

//-----------------------------------------------------------------------------
void ChannelEdges::invalidateResults()
{
  QWriteLocker lock(&m_dataMutex);
  if(!m_edgesAnalyzer->hasFinished())
  {
    m_edgesAnalyzer->abort();

    if ((m_edgesAnalyzer->thread() != this->thread()) && !m_edgesAnalyzer->thread()->wait(500))
    {
      m_edgesAnalyzer->thread()->terminate();
    }
  }

  if(!m_edgesCreator->hasFinished())
  {
    m_edgesCreator->abort();

    if ((m_edgesCreator->thread() != this->thread()) && !m_edgesCreator->thread()->wait(500))
    {
      m_edgesCreator->thread()->terminate();
    }
  }

  m_edges = nullptr;
  for(int i = 0; i < 6; ++i)
  {
    m_faces[i] = nullptr;
  }

  m_computedVolume     = 0;
  m_hasAnalizedChannel = false;
  m_hasCreatedEdges    = false;
  m_edgesAnalyzer      = std::make_shared<EdgesAnalyzer>(this, m_scheduler);
  m_edgesCreator       = std::make_shared<AdaptiveEdgesCreator>(this, m_scheduler);
}

//-----------------------------------------------------------------------------
void ChannelEdges::setAnalisysValues(bool useBounds, int color, int threshold)
{
  if((m_useDistanceToBounds != useBounds) || (m_backgroundColor != color) || (m_threshold != threshold))
  {
    m_useDistanceToBounds = useBounds;
    m_backgroundColor     = color;
    m_threshold           = threshold;

    invalidateResults();

    // this allows a re-evaluation of the values.
    if(!useBounds && color == -1)
    {
      analyzeChannel();
    }

    emit invalidated();
  }
}

//-----------------------------------------------------------------------------
bool ChannelEdges::useDistanceToBounds() const
{
  checkAnalysisData();

  return m_useDistanceToBounds;
}

//-----------------------------------------------------------------------------
int ChannelEdges::backgroundColor() const
{
  checkAnalysisData();

  return m_backgroundColor;
}

//-----------------------------------------------------------------------------
int ChannelEdges::threshold() const
{
  checkAnalysisData();

  return m_threshold;
}

//-----------------------------------------------------------------------------
void ChannelEdges::checkAnalysisData() const
{
  if(!m_hasAnalizedChannel)
  {
    const_cast<ChannelEdges *>(this)->analyzeChannel();

    m_analysisResultMutex.lock();
    m_analisysWait.wait(&m_analysisResultMutex);
    m_analysisResultMutex.unlock();
  }
}

//-----------------------------------------------------------------------------
void ChannelEdges::checkEdgesData() const
{
  if(!m_hasCreatedEdges)
  {
    const_cast<ChannelEdges *>(this)->computeAdaptiveEdges();

    m_edgesResultMutex.lock();
    m_edgesTask.wait(&m_edgesResultMutex);
    m_edgesResultMutex.unlock();
  }
}
