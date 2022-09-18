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
#include <Core/Analysis/Data/SkeletonData.h>
#include <Core/Analysis/Data/VolumetricDataUtils.hxx>
#include <Core/Utils/vtkPolyDataUtils.h>
#include <Core/Utils/SpatialUtils.hxx>

// VTK
#include <vtkCellArray.h>
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
#include <vtkImplicitPolyDataDistance.h>
#include <vtkCellArray.h>

// Qt
#include <QThread>

using namespace ESPINA;
using namespace ESPINA::Core;
using namespace ESPINA::Extensions;

const StackExtension::Type ChannelEdges::TYPE = "AdaptiveEdges";

const QString ChannelEdges::EDGES_FILE = "ChannelEdges.vtp";
const QString ChannelEdges::FACES_FILE = "ChannelFaces_%1.vtp";

using VTKReader = vtkSmartPointer<vtkGenericDataObjectReader>;
using VTKWriter = vtkSmartPointer<vtkGenericDataObjectWriter>;
using ComputedSegmentation = std::pair<unsigned int, unsigned long int>;

//-----------------------------------------------------------------------------
ChannelEdges::ChannelEdges(SchedulerSPtr                    scheduler,
                           const StackExtension::InfoCache &cache,
                           const State                     &state)
: StackExtension       {cache}
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
    bool ok{false};
    m_useDistanceToBounds = values[0].toInt(&ok);
    bool result = ok;
    m_backgroundColor     = values[1].toInt(&ok);
    result &= ok;
    m_threshold           = values[2].toInt(&ok);
    result &= ok;

    if(result)
    {
      m_hasAnalizedChannel  = true;
    }
    else
    {
      m_useDistanceToBounds = true;
      m_backgroundColor     = -1;
      m_threshold           = -1;
    }
  }
}

//-----------------------------------------------------------------------------
ChannelEdges::~ChannelEdges()
{
  invalidateResults();
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
  QReadLocker lock(&m_dataMutex);

  if(m_edgesAnalyzer->isRunning() || m_edgesAnalyzer->hasFinished()) return;

  m_edgesAnalyzer->setDescription(QObject::tr("Analyzing Edges: %1").arg(m_extendedItem->name()));

  Task::submit(m_edgesAnalyzer);
}

//-----------------------------------------------------------------------------
void ChannelEdges::computeAdaptiveEdges()
{
  checkAnalysisData();

  QReadLocker lock(&m_dataMutex);

  if(m_edgesCreator->isRunning() || m_edgesCreator->hasFinished()) return;

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

  QReadLocker lock(&m_dataMutex);

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

    double LB[3]{-1,-1,-1}, RT[3]{0,0,0};

    QReadLocker lock(&m_dataMutex);

    const unsigned int maxPoints = m_edges->GetNumberOfPoints();
    if(maxPoints > 0)
    {
      m_edges->GetPoint(std::min(slice*4+0, maxPoints -1), LB);
      m_edges->GetPoint(std::min(slice*4+2, maxPoints -1), RT);
    }

    Bounds bounds{LB[0], RT[0], RT[1], LB[1], LB[2], LB[2]};

    region = equivalentRegion<itkVolumeType>(origin, spacing, bounds);
  }

  auto stackRegion = readLockVolume(m_extendedItem->output())->itkRegion();
  region.Crop(stackRegion);

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

  bool computed = false;
  auto output = segmentation->output();

  auto segmentationPolyData = vtkSmartPointer<vtkPolyData>::New();

  if(hasMeshData(output))
  {
    segmentationPolyData->DeepCopy(writeLockMesh(output)->mesh());

    if(segmentationPolyData->GetNumberOfPoints() != 0)
    {
      for (int face = 0; face < 6; ++face)
      {
        const auto faceMesh = vtkSmartPointer<vtkPolyData>::New();

        {
          QMutexLocker lock(&m_distanceMutex);
          faceMesh->DeepCopy(m_faces[face]);
        }

        auto distanceFilter = vtkSmartPointer<vtkDistancePolyDataFilter>::New();
        distanceFilter->SignedDistanceOff();
        distanceFilter->SetInputData(0, segmentationPolyData);
        distanceFilter->SetInputData(1, faceMesh);
        distanceFilter->Update();

        distances[face] = distanceFilter->GetOutput()->GetPointData()->GetScalars()->GetRange()[0];
      }

      computed = true;
    }
  }
  else
  {
    if(hasSkeletonData(output))
    {
      segmentationPolyData->DeepCopy(writeLockSkeleton(output)->skeleton());

      // NOTE: Skeletons have no faces and vtkDistancePolyDataFilter returns an invalid result.
      //       Need to evaluate distance at each point.
      if(segmentationPolyData->GetNumberOfPoints() != 0)
      {
        for (int face = 0; face < 6; ++face)
        {
          const auto faceMesh = vtkSmartPointer<vtkPolyData>::New();

          {
            QMutexLocker lock(&m_distanceMutex);
            faceMesh->DeepCopy(m_faces[face]);
          }

          distances[face] = VTK_DOUBLE_MAX;

          auto implicit = vtkSmartPointer<vtkImplicitPolyDataDistance>::New();
          implicit->SetInput(faceMesh);

          for(vtkIdType i = 0; i < segmentationPolyData->GetNumberOfPoints(); ++i)
          {
            double pt[3];
            segmentationPolyData->GetPoint(i, pt);

            distances[face] = std::min(distances[face], std::fabs(implicit->EvaluateFunction(pt)));
          }
        }

        computed = true;
      }
    }
  }

  if(!computed)
  {
    qWarning() << tr("Unavailable information") << __FILE__ << __LINE__;
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
    if(m_edges) result->DeepCopy(m_edges);
  }

  return result;
}

//-----------------------------------------------------------------------------
Nm ChannelEdges::computedVolume()
{
  initializeEdges();

  QReadLocker lock(&m_dataMutex);

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

  {
    QWriteLocker lock(&m_dataMutex);
    m_invalidated = true;
  }

  emit invalidated();
}

//-----------------------------------------------------------------------------
void ChannelEdges::invalidateResults()
{
  QWriteLocker lock(&m_dataMutex);
  if(m_edgesAnalyzer->isRunning() && !m_edgesAnalyzer->hasFinished())
  {
    m_edgesAnalyzer->abort();

    if ((m_edgesAnalyzer->thread() != this->thread()) && !m_edgesAnalyzer->thread()->wait(500))
    {
      m_edgesAnalyzer->thread()->terminate();
    }

    m_analisysWait.wakeAll();
  }

  if(m_edgesCreator->isRunning() && !m_edgesCreator->hasFinished())
  {
    m_edgesCreator->abort();

    if ((m_edgesCreator->thread() != this->thread()) && !m_edgesCreator->thread()->wait(500))
    {
      m_edgesCreator->thread()->terminate();
    }

    m_edgesTask.wakeAll();
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
    else
    {
      QWriteLocker lock(&m_dataMutex);
      m_hasAnalizedChannel = true;
    }

    emit invalidated();
  }
}

//-----------------------------------------------------------------------------
bool ChannelEdges::useDistanceToBounds() const
{
  checkAnalysisData();

  QReadLocker lock(&m_dataMutex);

  return m_useDistanceToBounds;
}

//-----------------------------------------------------------------------------
int ChannelEdges::backgroundColor() const
{
  checkAnalysisData();

  QReadLocker lock(&m_dataMutex);

  return m_backgroundColor;
}

//-----------------------------------------------------------------------------
int ChannelEdges::threshold() const
{
  checkAnalysisData();

  QReadLocker lock(&m_dataMutex);

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
void ChannelEdges::checkEdgesData()
{
  if(!m_hasCreatedEdges || !m_faces[0] || !m_faces[1] || !m_faces[2] || !m_faces[3] || !m_faces[4] || !m_faces[5] || !m_edges)
  {
    if(!useDistanceToBounds())
    {
      const_cast<ChannelEdges *>(this)->computeAdaptiveEdges();

      m_edgesResultMutex.lock();
      m_edgesTask.wait(&m_edgesResultMutex);
      m_edgesResultMutex.unlock();
    }
    else
    {
      auto volumeBounds = readLockVolume(m_extendedItem->output())->bounds().bounds();
      createRectangularRegion(volumeBounds);

      QWriteLocker lock(&m_dataMutex);
      m_hasCreatedEdges = true;
    }
  }
}

//-----------------------------------------------------------------------------
void ChannelEdges::createRectangularRegion(const Bounds &bounds)
{
  QWriteLocker lock(&m_dataMutex);

  m_edges       = vtkSmartPointer<vtkPolyData>::New();
  auto points   = vtkSmartPointer<vtkPoints>::New();
  auto cells    = vtkSmartPointer<vtkCellArray>::New();

  auto left   = bounds[0];
  auto top    = bounds[2];
  auto front  = bounds[4];
  auto right  = bounds[1];
  auto bottom = bounds[3];
  auto back   = bounds[5];

  vtkIdType frontFace[4], leftFace[4] , topFace[4];
  vtkIdType backFace[4] , rightFace[4], bottomFace[4];

  // Front Face
  frontFace[0] = points->InsertNextPoint(left,  bottom, front );
  frontFace[1] = points->InsertNextPoint(left,  top,    front );
  frontFace[2] = points->InsertNextPoint(right, top,    front );
  frontFace[3] = points->InsertNextPoint(right, bottom, front );
  cells->InsertNextCell(4, frontFace);

  // Back Face
  backFace[0] = points->InsertNextPoint(left,  bottom, back);
  backFace[1] = points->InsertNextPoint(left,  top,    back);
  backFace[2] = points->InsertNextPoint(right, top,    back);
  backFace[3] = points->InsertNextPoint(right, bottom, back);
  cells->InsertNextCell(4, backFace);

  // Left Face
  leftFace[0] = frontFace[0];
  leftFace[1] = frontFace[1];
  leftFace[2] = backFace[1];
  leftFace[3] = backFace[0];
  cells->InsertNextCell(4, leftFace);

  // Right Face
  rightFace[0] = frontFace[2];
  rightFace[1] = frontFace[3];
  rightFace[2] = backFace[3];
  rightFace[3] = backFace[2];
  cells->InsertNextCell(4, rightFace);

  // Top Face
  topFace[0] = frontFace[1];
  topFace[1] = frontFace[2];
  topFace[2] = backFace[2];
  topFace[3] = backFace[1];
  cells->InsertNextCell(4, topFace);

  // Bottom Face
  bottomFace[0] = frontFace[3];
  bottomFace[1] = frontFace[0];
  bottomFace[2] = backFace[0];
  bottomFace[3] = backFace[3];
  cells->InsertNextCell(4, bottomFace);

  m_edges->SetPoints(points);
  m_edges->SetPolys(cells);

  // Left Face
  m_faces[0]      = vtkSmartPointer<vtkPolyData>::New();
  auto leftpoints = vtkSmartPointer<vtkPoints>::New();
  auto leftcells  = vtkSmartPointer<vtkCellArray>::New();

  leftFace[0] = leftpoints->InsertNextPoint(left,  bottom, front);
  leftFace[1] = leftpoints->InsertNextPoint(left,  top,    front);
  leftFace[2] = leftpoints->InsertNextPoint(left,  top,    back);
  leftFace[3] = leftpoints->InsertNextPoint(left,  bottom, back);
  leftcells->InsertNextCell(4, leftFace);

  m_faces[0]->SetPoints(leftpoints);
  m_faces[0]->SetPolys(leftcells);

  // Right face
  m_faces[1]       = vtkSmartPointer<vtkPolyData>::New();
  auto rightpoints = vtkSmartPointer<vtkPoints>::New();
  auto rightcells  = vtkSmartPointer<vtkCellArray>::New();

  rightFace[0] = rightpoints->InsertNextPoint(right, top,    front);
  rightFace[1] = rightpoints->InsertNextPoint(right, bottom, front);
  rightFace[2] = rightpoints->InsertNextPoint(right, bottom, back);
  rightFace[3] = rightpoints->InsertNextPoint(right, top,    back);
  rightcells->InsertNextCell(4, rightFace);

  m_faces[1]->SetPoints(rightpoints);
  m_faces[1]->SetPolys(rightcells);

  // Top Face
  m_faces[2]     = vtkSmartPointer<vtkPolyData>::New();
  auto toppoints = vtkSmartPointer<vtkPoints>::New();
  auto topcells  = vtkSmartPointer<vtkCellArray>::New();

  topFace[0] = toppoints->InsertNextPoint(left,  top,    front);
  topFace[1] = toppoints->InsertNextPoint(right, top,    front);
  topFace[2] = toppoints->InsertNextPoint(right, top,    back);
  topFace[3] = toppoints->InsertNextPoint(left,  top,    back);
  topcells->InsertNextCell(4, topFace);

  m_faces[2]->SetPoints(toppoints);
  m_faces[2]->SetPolys(topcells);

  // Bottom face
  m_faces[3]        = vtkSmartPointer<vtkPolyData>::New();
  auto bottompoints = vtkSmartPointer<vtkPoints>::New();
  auto bottomcells  = vtkSmartPointer<vtkCellArray>::New();

  bottomFace[0] = bottompoints->InsertNextPoint(right, bottom, front );
  bottomFace[1] = bottompoints->InsertNextPoint(left,  bottom, front );
  bottomFace[2] = bottompoints->InsertNextPoint(left,  bottom, back);
  bottomFace[3] = bottompoints->InsertNextPoint(right, bottom, back);
  bottomcells->InsertNextCell(4, bottomFace);

  m_faces[3]->SetPoints(bottompoints);
  m_faces[3]->SetPolys(bottomcells);

  // Front face
  m_faces[4]       = vtkSmartPointer<vtkPolyData>::New();
  auto frontpoints = vtkSmartPointer<vtkPoints>::New();
  auto frontcells  = vtkSmartPointer<vtkCellArray>::New();

  frontFace[0] = frontpoints->InsertNextPoint(left,  bottom, front );
  frontFace[1] = frontpoints->InsertNextPoint(left,  top,    front );
  frontFace[2] = frontpoints->InsertNextPoint(right, top,    front );
  frontFace[3] = frontpoints->InsertNextPoint(right, bottom, front );
  frontcells->InsertNextCell(4, frontFace);

  m_faces[4]->SetPoints(frontpoints);
  m_faces[4]->SetPolys(frontcells);

  // Back Face
  m_faces[5]      = vtkSmartPointer<vtkPolyData>::New();
  auto backpoints = vtkSmartPointer<vtkPoints>::New();
  auto backcells  = vtkSmartPointer<vtkCellArray>::New();

  backFace[0] = backpoints->InsertNextPoint(left,  bottom, back);
  backFace[1] = backpoints->InsertNextPoint(left,  top,    back);
  backFace[2] = backpoints->InsertNextPoint(right, top,    back);
  backFace[3] = backpoints->InsertNextPoint(right, bottom, back);
  backcells->InsertNextCell(4, backFace);

  m_faces[5]->SetPoints(backpoints);
  m_faces[5]->SetPolys(backcells);
}

//-----------------------------------------------------------------------------
bool ChannelEdges::isPointOnEdge(const NmVector3 point, const Nm tolerance)
{
  initializeEdges();

  bool result = false;
  auto bounds = m_extendedItem->bounds();

  if(contains(bounds, point))
  {
    if(useDistanceToBounds())
    {
      for(auto i: {0,1,2})
      {
        auto lower = point[i] - bounds[2*i];
        auto upper = bounds[(2*i)+1] - point[i];

        result |= lower >= 0 && lower < tolerance;
        result |= upper >= 0 && upper < tolerance;

        if(result) break;
      }
    }
    else
    {
      auto spacing    = m_extendedItem->output()->spacing();
      Nm halfZSpacing = spacing[2]/2.;
      auto sliceIndex = std::floor((point[2]+halfZSpacing - (bounds[4]+halfZSpacing))/spacing[2]);
      auto region     = sliceRegion(sliceIndex);
      auto maxSlice   = std::floor((bounds[5]+halfZSpacing)/spacing[2]) - 1; // 0 <-> N-1
      bool isInside   = true; // point inside X-Y frame.

      for(auto i: {0,1})
      {
        auto lowerPoint = (region.GetIndex(i)*spacing[i]) - spacing[i]/2.0;
        auto upperPoint = ((region.GetIndex(i) + region.GetSize(i))*spacing[i]) - spacing[i]/2.0;

        auto lower = point[i] - lowerPoint;
        auto upper = upperPoint - point[i];

        result |= lower >= 0 && lower < tolerance;
        result |= upper >= 0 && upper < tolerance;

        if(result) break;

        isInside &= (lowerPoint <= point[i]) && (point[i] <= upperPoint);
      }

      if(!result && isInside)
      {
        // check upper and lower Z slices of the stack.
        auto zTolerance = std::max(tolerance, spacing[2]);
        if(sliceIndex == 0) result |= (point[2] - bounds[4] < zTolerance);
        if(sliceIndex == maxSlice) result |= (bounds[5] - point[2] < zTolerance);
      }
    }
  }

  return result;
}
