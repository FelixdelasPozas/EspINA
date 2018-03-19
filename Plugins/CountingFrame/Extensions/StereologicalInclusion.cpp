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
#include <GUI/Utils/Format.h>
#include <Core/Analysis/Query.h>
#include <Core/Analysis/Channel.h>
#include <Core/Analysis/Segmentation.h>
#include <Core/Analysis/Data/VolumetricData.hxx>
#include <Core/Analysis/Data/Volumetric/SparseVolume.hxx>
#include <Core/Analysis/Category.h>
#include <Core/Utils/EspinaException.h>

// VTK
#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <itkImageRegionIterator.h>

// Qt
#include <QDebug>
#include <QApplication>

using namespace ESPINA;
using namespace ESPINA::Core;
using namespace ESPINA::Extensions;
using namespace ESPINA::CF;
using namespace ESPINA::GUI::Utils::Format;

const SegmentationExtension::Type StereologicalInclusion::TYPE = "StereologicalInclusion";

//------------------------------------------------------------------------
SegmentationExtension::InformationKey StereologicalInclusion::cfKey(CountingFrame *cf) const
{
  return createKey(tr("Inc. %1 CF").arg(cf->id()));
}

//------------------------------------------------------------------------
StereologicalInclusion::StereologicalInclusion(const Extension< Segmentation >::InfoCache& infoCache)
: SegmentationExtension{infoCache}
, m_isUpdated          {false}
, m_isExcluded         {false}
{
}

//------------------------------------------------------------------------
State StereologicalInclusion::state() const
{
  return State();
}

//------------------------------------------------------------------------
Snapshot StereologicalInclusion::snapshot() const
{
  return Snapshot();
}

//------------------------------------------------------------------------
SegmentationExtension::TypeList StereologicalInclusion::dependencies() const
{
  TypeList dependencies;

  dependencies << ChannelEdges::TYPE;

  return dependencies;
}

//------------------------------------------------------------------------
SegmentationExtension::InformationKeyList StereologicalInclusion::availableInformation() const
{
  QMutexLocker lock(&m_mutex);

  return m_keys;
}

//------------------------------------------------------------------------
QVariant StereologicalInclusion::cacheFail(const InformationKey& key) const
{
  //evaluateCountingFrames();

  return cachedInfo(key);
}

//------------------------------------------------------------------------
void StereologicalInclusion::onExtendedItemSet(Segmentation* segmentation)
{
  connect(segmentation, SIGNAL(outputModified()),
          this,         SLOT(onOutputModified()));
}

//------------------------------------------------------------------------
QString StereologicalInclusion::toolTipText() const
{
  QString tooltip;

  {
    InformationKeyList keys;
    {
      QMutexLocker lock(&m_mutex);
      keys = m_keys;
      keys.detach();
    }

    for(auto key: keys)
    {
      QString description = cachedInfo(key).toBool()?
      "<font color=\"green\">" + tr("Included in %1 Counting Frame"  ).arg(key.value()) + "</font>":
      "<font color=\"red\">"   + tr("Excluded from %1 Counting Frame").arg(key.value()) + "</font>";
      tooltip = tooltip.append(createTable(":/apply.svg", description));
    }
  }

  return tooltip;
}

//------------------------------------------------------------------------
void StereologicalInclusion::addCountingFrame(CountingFrame* cf)
{
  QMutexLocker lock(&m_mutex);

  auto channels = QueryContents::channels(m_extendedItem);
  bool validChannel = false;
  for(auto channel: channels)
  {
    if(channel.get() == cf->channel())
    {
      validChannel = true;
      break;
    }
  }

  if (!m_exclusionCFs.contains(cf) && validChannel)
  {
    m_exclusionCFs[cf] = false;
    m_cfIds[cf]        = cf->id();
    m_isUpdated        = false;
    m_keys            << cfKey(cf);

    connect(cf,   SIGNAL(modified(CountingFrame *)),
            this, SLOT(onCountingFrameModified(CountingFrame *)));
  }
}

//------------------------------------------------------------------------
void StereologicalInclusion::removeCountingFrame(CountingFrame* cf)
{
  QMutexLocker lock(&m_mutex);

  if (m_exclusionCFs.contains(cf))
  {
    m_exclusionCFs.remove(cf);
    m_cfIds.remove(cf);
    m_keys.removeOne(cfKey(cf));

    disconnect(cf,   SIGNAL(modified(CountingFrame *)),
               this, SLOT(onCountingFrameModified(CountingFrame *)));
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
  // NOTE: this method could trigger a modification in the output of the extended item, causing an endless loop.
  // Disconnecting signals fixes that.

  Q_ASSERT(m_extendedItem);

  disconnect(m_extendedItem, SIGNAL(outputModified()),
             this,           SLOT(onOutputModified()));

  checkSampleCountingFrames();

  if (!m_isUpdated && !m_exclusionCFs.isEmpty())
  {
    for (auto cf : m_exclusionCFs.keys())
    {
      evaluateCountingFrame(cf);
    }

    m_isUpdated = true;
  }

  connect(m_extendedItem, SIGNAL(outputModified()),
          this,           SLOT(onOutputModified()));
}

//------------------------------------------------------------------------
void StereologicalInclusion::evaluateCountingFrame(CountingFrame* cf)
{
  if(!m_exclusionCFs.keys().contains(cf)) return;

  auto key = cfKey(cf);

  updateInfoCache(key.value(), QVariant());

  // Compute CF's exclusion value
  bool excluded = isExcludedByCountingFrame(cf);

  QVariant info;

  info.setValue<int>(excluded ? 0 : 1);

  {
    QMutexLocker lock(&m_mutex);

    updateInfoCache(key.value(), info);

    m_exclusionCFs[cf] = excluded;

    // Update segmentation's exclusion value
    excluded = true;

    int i = 0;
    auto countingFrames = m_exclusionCFs.keys();
    while (excluded && i < countingFrames.size())
    {
      excluded = excluded && m_exclusionCFs[countingFrames[i]];
      i++;
    }

    m_isExcluded = excluded;
  }
}

//------------------------------------------------------------------------
bool StereologicalInclusion::isExcludedByCountingFrame(CountingFrame* cf)
{
  //qDebug() << "Checking Counting Frame Exclusion";
  auto segmentationCategory = m_extendedItem->category()->classificationName();

  if (!segmentationCategory.startsWith(cf->categoryConstraint()))
  {
    return true;
  }

  auto output       = m_extendedItem->output();
  auto inputBB      = output->bounds();
  auto spacing      = output->spacing();
  auto region       = cf->innerFramePolyData();
  auto regionPoints = region->GetPoints();

  auto pointBounds = [] (vtkPoints *points)
  {
    return Bounds{points->GetBounds()};
  };

  Bounds regionBB = pointBounds(regionPoints);

  // If there is no intersection (nor is inside), then it is excluded
  if (!intersect(inputBB, regionBB, spacing))
  {
    return true;
  }

  // Fast check of outside sides (outer cf sides that extends to infinite)
  for(auto i: {1,3,5})
  {
    if(inputBB[i] > regionBB[i]) return true;
  }

  // NOTE: CF slices != stack slices.
  for (vtkIdType i = 0; i < regionPoints->GetNumberOfPoints(); i += 4)
  {
    auto slicePoints = vtkSmartPointer<vtkPoints>::New();
    for (int j = 0; j < 4; j++)
    {
      double point[3];
      regionPoints->GetPoint(i + j, point);
      slicePoints->InsertNextPoint(point);
    }

    auto sliceBounds = pointBounds(slicePoints);
    if(i == 0) sliceBounds[4] -= (spacing[2]/2.0);
    if(i == 0) sliceBounds[5] += (spacing[2]/2.0);
    else       sliceBounds[5] += spacing[2];

    if(sliceBounds.areValid())
    {
      sliceBounds.setLowerInclusion(true);
      sliceBounds.setUpperInclusion(true);

      if (intersect(inputBB, sliceBounds, spacing))
      {
        itkVolumeType::Pointer sliceImage = nullptr;

        auto sliceIntersection = intersection(inputBB, sliceBounds);
        Q_ASSERT(sliceIntersection.areValid());

        try
        {
          sliceImage = readLockVolume(m_extendedItem->output())->itkImage(sliceIntersection);
        }
        catch(Core::Utils::EspinaException &e)
        {

          continue;
        }

        auto minBounds = minimalBounds<itkVolumeType>(sliceImage, SEG_BG_VALUE);
        if(!minBounds.areValid()) continue; // means that the part inside the CF slice is empty (no voxels == SEG_VOXEL_VALUE)

        for(auto i: {1,3})
        {
          // segmentation can have several "parts" and if one is outside then is out
          if(minBounds[i] == sliceBounds[i])
          {
            return true;
          }
        }

        return false; // intersection has voxels and doesn't touch red faces, is completely inside or touches green faces.
      }
    }
  }

  return true;
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
    qWarning() << "Counting Frame<evaluateCountingFrames>: Tiling mode not supported";
  }
  else
  {
    if (!samples.isEmpty())
    {
      auto sample = samples.first();

      for(auto channel : QueryContents::channels(sample))
      {
        auto extensions = channel->readOnlyExtensions();

        if (extensions->hasExtension(CountingFrameExtension::TYPE))
        {
          auto extension = extensions->get<CountingFrameExtension>();

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
void StereologicalInclusion::onCountingFrameModified(CountingFrame *cf)
{
  QMutexLocker lock(&m_mutex);

  if(m_exclusionCFs.keys().contains(cf))
  {
    auto oldIdKey = tr("Inc. %1 CF").arg(m_cfIds[cf]);
    auto newIdKey = tr("Inc. %1 CF").arg(cf->id());

    m_cfIds[cf] = cf->id();

    if(m_infoCache.keys().contains(oldIdKey))
    {
      m_infoCache.insert(newIdKey, m_infoCache[oldIdKey]);
      m_infoCache.remove(oldIdKey);
    }

    m_keys.removeOne(createKey(oldIdKey));
    m_keys << createKey(newIdKey);
  }
}

//------------------------------------------------------------------------
void StereologicalInclusion::onOutputModified()
{
  m_isUpdated = false;

  evaluateCountingFrames();
}

//------------------------------------------------------------------------
bool StereologicalInclusion::validData(const OutputSPtr output) const
{
  return true;
}

//------------------------------------------------------------------------
SegmentationExtension::InformationKeyList StereologicalInclusion::readyInformation() const
{
  QMutexLocker lock(&m_mutex);

  InformationKeyList keys;

  for (auto key : SegmentationExtension::readyInformation())
  {
    if(!m_keys.contains(key))
    {
      m_infoCache.remove(key);
    }
    else
    {
      keys << key;
    }
  }

  return keys;
}
