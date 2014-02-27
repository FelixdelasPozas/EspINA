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

#include "StereologicalInclusion.h"

#include "CountingFrameExtension.h"
#include "CountingFrames/CountingFrame.h"
#include <Extensions/EdgeDistances/ChannelEdges.h>
#include <Extensions/ExtensionUtils.h>
#include <GUI/Utils/Conditions.h>
#include <Core/Analysis/Query.h>
#include <Core/Analysis/Channel.h>
#include <Core/Analysis/Segmentation.h>
#include <Core/Analysis/Data/VolumetricData.h>
#include <Core/Analysis/Category.h>

#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <itkImageRegionIterator.h>

#include <QDebug>
#include <QApplication>

using namespace EspINA;
using namespace EspINA::CF;

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
: SegmentationExtension(infoCache)
, m_isInitialized(false)
, m_isUpdated(false)
, m_isExcluded(false)
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
  } else
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
//     connect(cf, SIGNAL(modified(CountingFrame*)),
//             this, SLOT(evaluateCountingFrame(CountingFrame*)));
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
//     disconnect(cf, SIGNAL(modified(CountingFrame*)),
//                this, SLOT(evaluateCountingFrame(CountingFrame*)));
    m_exclusionCFs.remove(cf);
    m_excludedByCF.remove(cf->id());
    m_isUpdated = false;
  }
}

//------------------------------------------------------------------------
bool StereologicalInclusion::isExcluded() const
{
//   if (!m_isInitialized)
//   {
//     const_cast<StereologicalInclusion *>(this)->evaluateCountingFrames();
//   }
//
  return m_isExcluded;
}

//------------------------------------------------------------------------
void StereologicalInclusion::evaluateCountingFrames()
{
  checkSampleCountingFrames();

  if (!m_isUpdated)
  {
    if (!m_extendedItem)
      return;

    if (m_excludedByCF.isEmpty())
    {
      m_isExcluded = isOnEdge();
    } else
    {
      for (auto cf : m_exclusionCFs.keys())
      {
        evaluateCountingFrame(cf);
      }
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
    //info.setValue<QString>("Excluded");
    info.setValue<int>(0);
  }
  else
  {
    //info.setValue<QString>("Included");
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
  auto segmentationCategory = m_extendedItem->category()->classificationName();

  if (!segmentationCategory.startsWith(cf->categoryConstraint()))
    return true;

  Bounds inputBB = m_extendedItem->output()->bounds();
  //qDebug() << "Input:" << inputBB.toString();

  auto          region       = cf->region();
  vtkPoints    *regionPoints = region->GetPoints();
  vtkCellArray *regionFaces  = region->GetPolys();
  vtkCellData  *faceData     = region->GetCellData();

  auto pointBounds = [] (vtkPoints *points) {
    Bounds bounds;
    for (int i = 0; i < 6; ++i)
      bounds[i] = points->GetBounds()[i];
    return bounds;
  };

  Bounds regionBB = pointBounds(regionPoints);
  //qDebug() << "Region:" << regionBB.toString();

  // If there is no intersection (nor is inside), then it is excluded
  if (!intersect(inputBB, regionBB))
    return true;

  bool collisionDected = false;
  // Otherwise, we have to test all faces collisions
  int numOfCells = regionFaces->GetNumberOfCells();
  regionFaces->InitTraversal();
  for(int f=0; f < numOfCells; f++)
  {
    vtkIdType npts, *pts;
    regionFaces->GetNextCell(npts, pts);

    vtkSmartPointer<vtkPoints> facePoints = vtkSmartPointer<vtkPoints>::New();
    for (int i=0; i < npts; i++)
      facePoints->InsertNextPoint(regionPoints->GetPoint(pts[i]));

    Bounds faceBB = pointBounds(facePoints);
    //qDebug() << "Face:" << faceBB.toString();
    //qDebug() << " - intersect:" << intersect(inputBB, faceBB);
    //qDebug() << " - interscetion:" << intersection(inputBB, faceBB).toString();
    //qDebug() << " - type:" << faceData->GetScalars()->GetComponent(f, 0);

    if (intersect(inputBB, faceBB) && isRealCollision(intersection(inputBB, faceBB)))
    {
      if (faceData->GetScalars()->GetComponent(f,0) == cf->EXCLUSION_FACE)
        return true;
      collisionDected = true;
    }
  }

  if (collisionDected)
    return false;

  // If no collision was detected we have to check for inclusion
  for (int p=0; p + 7 < regionPoints->GetNumberOfPoints(); p +=4)
  {
    vtkSmartPointer<vtkPoints> slicePoints = vtkSmartPointer<vtkPoints>::New();
    for (int i=0; i < 8; i++)
      slicePoints->InsertNextPoint(regionPoints->GetPoint(p+i));

    Bounds sliceBB = pointBounds(slicePoints);
    if (intersect(inputBB, sliceBB) && isRealCollision(intersection(inputBB, sliceBB)))
      return false;//;
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
    Nm   threshold = 1.0;

    auto channels = Query::channels(m_extendedItem);
    if (channels.size() > 1)
    {
      qWarning() << "Tailing not supported by Stereological Inclusion Extension";
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
bool StereologicalInclusion::isRealCollision(const Bounds& interscetion)
{
  using ImageIterator = itk::ImageRegionIterator<itkVolumeType>;

  auto volume = volumetricData(m_extendedItem->output());
  auto image = volume->itkImage(interscetion);

  ImageIterator it = ImageIterator(image, image->GetLargestPossibleRegion());
  it.GoToBegin();
  while (!it.IsAtEnd()) {
    if (it.Get() != volume->backgroundValue())
      return true;
    ++it;
  }

  return false;
}

//------------------------------------------------------------------------
void StereologicalInclusion::checkSampleCountingFrames()
{
  auto samples = Query::samples(m_extendedItem);

  if (samples.size() > 1)
  {
    qWarning() << "Counting Frame<evaluateCountingFrames>: Tiling mode not suppoerted";
  } else if (!samples.isEmpty())
  {
    auto sample = samples.first();

    for(auto channel : Query::channels(sample))
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

//------------------------------------------------------------------------
StereologicalInclusionSPtr EspINA::CF::stereologicalInclusion(SegmentationExtensionSPtr extension)
{
  return std::dynamic_pointer_cast<StereologicalInclusion>(extension);
}