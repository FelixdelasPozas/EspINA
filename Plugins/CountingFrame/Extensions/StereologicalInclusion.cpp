/*
    
    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>

    This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of|| m_ex
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.m_exclusionCFs.is

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

// Plugin
#include "StereologicalInclusion.h"
#include "CountingFrameExtension.h"
#include "CountingFrames/CountingFrame.h"

// ESPINA
#include <Extensions/EdgeDistances/ChannelEdges.h>
#include <Extensions/ExtensionUtils.h>
#include <GUI/Utils/Conditions.h>
#include <Core/Analysis/Query.h>
#include <Core/Analysis/Channel.h>
#include <Core/Analysis/Segmentation.h>
#include <Core/Analysis/Data/VolumetricData.hxx>
#include <Core/Analysis/Category.h>

// VTK
#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <itkImageRegionIterator.h>

// Qt
#include <QDebug>
#include <QApplication>

using namespace ESPINA;
using namespace ESPINA::CF;

const SegmentationExtension::InfoTag EDGE_TAG = "Touch Edge";

const SegmentationExtension::Type    StereologicalInclusion::TYPE     = "StereologicalInclusion";
//const SegmentationExtension::InfoTag StereologicalInclusion::EXCLUDED = "Excluded from CF";

const QString StereologicalInclusion::FILE = StereologicalInclusion::TYPE + "/StereologicalInclusion.csv";

const std::string FILE_VERSION = StereologicalInclusion::TYPE.toStdString() + " 1.0\n";
const char SEP = ';';

//------------------------------------------------------------------------
SegmentationExtension::InfoTag StereologicalInclusion::cfTag(CountingFrame *cf)
{
  return tr("Inc. %1 CF").arg(cf->id());
}

//------------------------------------------------------------------------
StereologicalInclusion::StereologicalInclusion(const Extension< Segmentation >::InfoCache& infoCache)
: SegmentationExtension{infoCache}
, m_isInitialized      {false}
, m_isUpdated          {false}
, m_isExcluded         {false}
{
}

//------------------------------------------------------------------------
StereologicalInclusion::~StereologicalInclusion()
{
}

//------------------------------------------------------------------------
State StereologicalInclusion::state() const
{
  State state;

  return state;
}

//------------------------------------------------------------------------
Snapshot StereologicalInclusion::snapshot() const
{
  Snapshot snapshot;

  return snapshot;
}

//------------------------------------------------------------------------
SegmentationExtension::TypeList StereologicalInclusion::dependencies() const
{
  TypeList dependencies;

  //dependencies << EdgeDistance::TYPE;

  return dependencies;
}

//------------------------------------------------------------------------
SegmentationExtension::InfoTagList StereologicalInclusion::availableInformations() const
{
  InfoTagList tags;

  tags << EDGE_TAG;
  for (auto cf : m_exclusionCFs.keys())
  {
    tags << cfTag(cf);
  }

  return tags;
}

//------------------------------------------------------------------------
QVariant StereologicalInclusion::cacheFail(const QString& tag) const
{
  if (EDGE_TAG == tag)
  {
    isOnEdge();
  }
  else
  {
    //evaluateCountingFrames();
  }

  return cachedInfo(tag);
}

//------------------------------------------------------------------------
void StereologicalInclusion::onExtendedItemSet(Segmentation* segmentation)
{

}

//------------------------------------------------------------------------
QString StereologicalInclusion::toolTipText() const
{
  QString tooltip;

  if (isOnEdge())
  {
    QString description = "<font color=\"red\">"   + tr("Touches Stack Edge") + "</font>";
    tooltip = tooltip.append(condition(":/apply.svg", description));
  }

  for(auto cf : m_exclusionCFs.keys())
  {
    QString description = information(cfTag(cf)).toBool()?
    "<font color=\"green\">" + tr("Included in %1 Counting Frame"  ).arg(cf->id()) + "</font>":
    "<font color=\"red\">"   + tr("Excluded from %1 Counting Frame").arg(cf->id()) + "</font>";
    tooltip = tooltip.append(condition(":/apply.svg", description));
  }

  return tooltip;
}

//------------------------------------------------------------------------
void StereologicalInclusion::addCountingFrame(CountingFrame* cf)
{
  QMutexLocker lock(&m_mutex);

  if (!m_exclusionCFs.contains(cf))
  {
    m_excludedByCF[cf->id()] = false; // Everybody is innocent until proven guilty
    m_exclusionCFs[cf] = false;
    m_isUpdated = false;
  }
}

//------------------------------------------------------------------------
void StereologicalInclusion::removeCountingFrame(CountingFrame* cf)
{
  QMutexLocker lock(&m_mutex);

  if (m_exclusionCFs.contains(cf))
  {
    m_exclusionCFs.remove(cf);
    m_excludedByCF.remove(cf->id());
    m_isUpdated = false;
  }
}

//------------------------------------------------------------------------
bool StereologicalInclusion::isExcluded()
{
  if(!m_isUpdated)
  {
    evaluateCountingFrames();
  }
  return m_isExcluded;
}

//------------------------------------------------------------------------
void StereologicalInclusion::evaluateCountingFrames()
{
  Q_ASSERT(m_extendedItem);

  checkSampleCountingFrames();

  if (!m_isUpdated && !m_exclusionCFs.isEmpty())
  {
    for (auto cf : m_exclusionCFs.keys())
    {
      evaluateCountingFrame(cf);
    }

    m_isInitialized = true;
    m_isUpdated = true;
  }
}

//------------------------------------------------------------------------
void StereologicalInclusion::evaluateCountingFrame(CountingFrame* cf)
{
  auto tag = cfTag(cf);

  updateInfoCache(tag, QVariant());

  // Compute CF's exclusion value
  bool excluded = isExcludedByCountingFrame(cf);

  QVariant info;
  if (excluded)
  {
    info.setValue<int>(0);
  }
  else
  {
    info.setValue<int>(1);
  }

  {
    QMutexLocker lock(&m_mutex);

    updateInfoCache(tag, info);

    m_exclusionCFs[cf] = excluded;

    m_excludedByCF[cf->id()] = excluded;

    // Update segmentation's exclusion value
    excluded = true;

    int i = 0;
    CountingFrameList countingFrames = m_exclusionCFs.keys();
    while (excluded && i < countingFrames.size())
    {
      excluded = excluded && m_exclusionCFs[countingFrames[i]];
      i++;
    }

    m_isExcluded = excluded;// || isOnEdge();
  }
}

//------------------------------------------------------------------------
bool StereologicalInclusion::isExcludedByCountingFrame(CountingFrame* cf)
{
  //qDebug() << "Checking Counting Frame Exclusion";
  auto segmentationCategory = m_extendedItem->category()->classificationName();

  if (!segmentationCategory.startsWith(cf->categoryConstraint()))
    return true;

  auto output  = m_extendedItem->output();
  auto inputBB = output->bounds();
  auto spacing = output->spacing();
//   qDebug() << "InputBB:" << inputBB;

  auto region       = cf->polyData();
  auto regionPoints = region->GetPoints();
  auto regionFaces  = region->GetPolys();
  auto faceData     = region->GetCellData();

  //qDebug() << "Checking Counting Frame Exclusion: CF Read";

  auto pointBounds = [] (vtkPoints *points)
  {
    Bounds bounds;
    double values[6];
    points->GetBounds(values);
    for (int i = 0; i < 6; ++i)
    {
      bounds[i] = values[i];
    }
    return bounds;
  };

  Bounds regionBB = pointBounds(regionPoints);
  //qDebug() << "Region:" << regionBB.toString();

  // If there is no intersection (nor is inside), then it is excluded
  if (!intersect(inputBB, regionBB, spacing))
    return true;

  bool collisionDected = false;
  // Otherwise, we have to test all faces collisions
  vtkIdType numOfCells   = regionFaces->GetNumberOfCells();
  vtkIdType cellLocation = 0;
  for(vtkIdType f = 0; f < numOfCells; ++f)
  {
    vtkIdType numPoints, *pointIds;
    regionFaces->GetCell(cellLocation, numPoints, pointIds);
    cellLocation += 1 + numPoints;

    auto facePoints = vtkSmartPointer<vtkPoints>::New();
    for (vtkIdType p=0; p < numPoints; ++p)
    {
      double point[3];
      regionPoints->GetPoint(pointIds[p], point);
      facePoints->InsertNextPoint(point);
    }

    Bounds faceBB = pointBounds(facePoints);
//     if (f == 0 || f == numOfCells -1)
//     {
//       qDebug() << "Face:"  << faceBB.toString();
//       qDebug() << " - intersect:" << intersect(inputBB, faceBB, spacing);
//       qDebug() << " - interscetion:" << intersection(inputBB, faceBB, spacing).toString();
//       qDebug() << " - type:" << faceData->GetScalars()->GetComponent(f, 0);
//     }

    if (intersect(inputBB, faceBB, spacing) && isRealCollision(intersection(inputBB, faceBB, spacing)))
    {
      if (faceData->GetScalars()->GetComponent(f,0) == cf->EXCLUSION_FACE)
        return true;
      collisionDected = true;
    }
  }

  if (collisionDected)
  {
    return false;
  }

  // If no collision was detected we have to check for inclusion
  for (vtkIdType p = 0; p + 7 < regionPoints->GetNumberOfPoints(); p +=4)
  {
    auto slicePoints = vtkSmartPointer<vtkPoints>::New();
    for (int i=0; i < 8; i++)
    {
      double point[3];
      regionPoints->GetPoint(p+i, point);
      slicePoints->InsertNextPoint(point);
    }

    Bounds sliceBB = pointBounds(slicePoints);
    if (intersect(inputBB, sliceBB, spacing) && isRealCollision(intersection(inputBB, sliceBB, spacing)))
    {
      return false;
    }
  }

  // If no internal collision was detected, then the input was indeed outside our
  // bounding region
  return true;
}

//------------------------------------------------------------------------
bool StereologicalInclusion::isOnEdge() const
{
  bool isOnEdge  = false;

  if (cachedInfo(EDGE_TAG).isValid())
  {
    isOnEdge = cachedInfo(EDGE_TAG).toBool();
  }
  else
  {
    Nm threshold = 1.0;

    auto channels = QueryRelations::channels(m_extendedItem);

    if(channels.empty())
    {
      qWarning() << "Segmentation" << m_extendedItem->name() << "is not related to any channel, cannot get edges information.";
    }

    if (channels.size() > 1)
    {
      qWarning() << "Tiling not supported by Stereological Inclusion Extension";
    }
    else if (channels.size() == 1)
    {
      auto channel   = channels.first();
      auto extension = channel->extension(ChannelEdges::TYPE);

      auto edgesExtension = std::dynamic_pointer_cast<ChannelEdges>(extension);

      Nm distances[6];
      if (edgesExtension->useDistanceToBounds())
      {
        edgesExtension->distanceToBounds(m_extendedItem, distances);
      }
      else
      {
        edgesExtension->distanceToEdges(m_extendedItem, distances);
      }

      for(int i = 0; i < 6; ++i)
      {
        isOnEdge |= distances[i] < threshold;
      }
    }

    updateInfoCache(EDGE_TAG, isOnEdge?1:0);
  }

  return isOnEdge;
}

//------------------------------------------------------------------------
bool StereologicalInclusion::isRealCollision(const Bounds& collisionBounds)
{
  using ImageIterator = itk::ImageRegionIterator<itkVolumeType>;

  auto output = m_extendedItem->output();

  if (hasVolumetricData(output))
  {
    auto volume = readLockVolume(output);
    auto bounds = intersection(volume->bounds(), collisionBounds, volume->spacing());

    if (bounds.areValid())
    {
      auto image  = volume->itkImage(bounds);

      auto it = ImageIterator(image, image->GetLargestPossibleRegion());
      it.GoToBegin();
      while (!it.IsAtEnd())
      {
        if (it.Get() != volume->backgroundValue()) return true;
        ++it;
      }
    }
  }
  else
  {
    // TODO: Detect collision using other techniques
    //       (possibly collision detection should be part of the data API)
  }

  return false;
}

//------------------------------------------------------------------------
bool StereologicalInclusion::hasCountingFrames() const
{
  return m_exclusionCFs.size() != 0;
}

//------------------------------------------------------------------------
void StereologicalInclusion::checkSampleCountingFrames()
{
  auto samples = QueryContents::samples(m_extendedItem);

  if (samples.size() > 1)
  {
    qWarning() << "Counting Frame<evaluateCountingFrames>: Tiling mode not suppoerted";
  }
  else
  {
    if (!samples.isEmpty())
    {
      auto sample = samples.first();

      for(auto channel : QueryContents::channels(sample))
      {
        if (channel->hasExtension(CountingFrameExtension::TYPE))
        {
          auto extension = retrieveExtension<CountingFrameExtension>(channel);

          for (auto cf : extension->countingFrames())
          {
            addCountingFrame(cf);
          }
        }
      }
    }
  }
}

//------------------------------------------------------------------------
StereologicalInclusionSPtr ESPINA::CF::stereologicalInclusion(SegmentationExtensionSPtr extension)
{
  return std::dynamic_pointer_cast<StereologicalInclusion>(extension);
}
