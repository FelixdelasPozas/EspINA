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
#include "EdgeDistance.h"
#include "ChannelEdges.h"
#include <Extensions/ExtensionUtils.h>
#include <Core/Analysis/Channel.h>
#include <Core/Analysis/Segmentation.h>
#include <Core/Analysis/Query.h>
#include <Core/Factory/CoreFactory.h>
#include <GUI/Utils/Format.h>

// Qt
#include <QDebug>

using namespace ESPINA;
using namespace ESPINA::Core;
using namespace ESPINA::Extensions;

const SegmentationExtension::Type EdgeDistance::TYPE = "EdgeDistance";

const SegmentationExtension::Key EdgeDistance::LEFT_DISTANCE   = "Left Distance";
const SegmentationExtension::Key EdgeDistance::TOP_DISTANCE    = "Top Distance";
const SegmentationExtension::Key EdgeDistance::FRONT_DISTANCE  = "Front Distance";
const SegmentationExtension::Key EdgeDistance::RIGHT_DISTANCE  = "Right Distance";
const SegmentationExtension::Key EdgeDistance::BOTTOM_DISTANCE = "Bottom Distance";
const SegmentationExtension::Key EdgeDistance::BACK_DISTANCE   = "Back Distance";
const SegmentationExtension::Key EdgeDistance::TOUCH_EDGES     = "Touch Edge";

//-----------------------------------------------------------------------------
EdgeDistance::EdgeDistance(CoreFactory *factory, const SegmentationExtension::InfoCache& cache, const State& state)
: SegmentationExtension{cache}
, m_factory            {factory}
{
  Q_ASSERT(factory);
}

//-----------------------------------------------------------------------------
State EdgeDistance::state() const
{
  return State();
}

//-----------------------------------------------------------------------------
Snapshot EdgeDistance::snapshot() const
{
  return Snapshot();
}

//-----------------------------------------------------------------------------
const SegmentationExtension::InformationKeyList EdgeDistance::availableInformation() const
{
  InformationKeyList keys;

  keys << createKey(LEFT_DISTANCE);
  keys << createKey(RIGHT_DISTANCE);
  keys << createKey(TOP_DISTANCE);
  keys << createKey(BOTTOM_DISTANCE);
  keys << createKey(FRONT_DISTANCE);
  keys << createKey(BACK_DISTANCE);
  keys << createKey(TOUCH_EDGES);

  return keys;
}

//------------------------------------------------------------------------
QVariant EdgeDistance::cacheFail(const InformationKey& key) const
{
  if(key.value() == TOUCH_EDGES)
  {
    isOnEdge();
  }
  else
  {
    updateDistances();
  }

  return cachedInfo(key);
}

//-----------------------------------------------------------------------------
void EdgeDistance::edgeDistance(Nm distances[6]) const
{
  QStringList labels{LEFT_DISTANCE, RIGHT_DISTANCE, TOP_DISTANCE, BOTTOM_DISTANCE, FRONT_DISTANCE, BACK_DISTANCE};
  for(auto label: labels)
  {
    auto info = information(createKey(label));
    bool ok = false;
    auto conversion = info.toDouble(&ok);
    auto position = labels.indexOf(label);

    if(!ok || (info.canConvert(QVariant::String) && info.toString() == "Unavailable"))
    {
      distances[position] = -1;
    }
    else
    {
      distances[position] = conversion;
    }
  }
}

//-----------------------------------------------------------------------------
void EdgeDistance::updateDistances() const
{
  // Prevent updating if all available information is already computed
  if (readyInformation().size() < 6)
  {
    QMutexLocker lock(&m_mutex);

    auto channels = QueryRelations::channels(m_extendedItem);

    if (channels.size() == 1)
    {
      Nm distances[6];
      auto channel = channels.first();

      auto edgesExtension = retrieveOrCreateStackExtension<ChannelEdges>(channel, m_factory);
      if (edgesExtension->useDistanceToBounds())
      {
        edgesExtension->distanceToBounds(m_extendedItem, distances);
      }
      else
      {
        edgesExtension->distanceToEdges(m_extendedItem, distances);
      }

      updateInfoCache(LEFT_DISTANCE  , distances[0]);
      updateInfoCache(RIGHT_DISTANCE , distances[1]);
      updateInfoCache(TOP_DISTANCE   , distances[2]);
      updateInfoCache(BOTTOM_DISTANCE, distances[3]);
      updateInfoCache(FRONT_DISTANCE , distances[4]);
      updateInfoCache(BACK_DISTANCE  , distances[5]);
    }
    else
    {
      auto text = tr("Unavailable");
      updateInfoCache(LEFT_DISTANCE  , text);
      updateInfoCache(RIGHT_DISTANCE , text);
      updateInfoCache(TOP_DISTANCE   , text);
      updateInfoCache(BOTTOM_DISTANCE, text);
      updateInfoCache(FRONT_DISTANCE , text);
      updateInfoCache(BACK_DISTANCE  , text);
    }
  }
}

//-----------------------------------------------------------------------------
const QString EdgeDistance::toolTipText() const
{
  QString tooltip;

  if (m_infoCache.contains(TOUCH_EDGES) && isOnEdge())
  {
    QString description = "<font color=\"red\">" + tr("Touches Stack Edge") + "</font>";
    tooltip = tooltip.append(GUI::Utils::Format::createTable(":/touchStackEdge.svg", description));
  }

  return tooltip;
}

//-----------------------------------------------------------------------------
bool EdgeDistance::isOnEdge() const
{
  bool isOnEdge  = false;

  if (m_infoCache.contains(TOUCH_EDGES) && m_infoCache[TOUCH_EDGES].isValid())
  {
    isOnEdge = m_infoCache[TOUCH_EDGES].toBool();
  }
  else
  {
    auto channels = QueryRelations::channels(m_extendedItem);

    if(channels.empty())
    {
      qWarning() << "Segmentation" << m_extendedItem->name() << "is not related to any stack, cannot get edges information.";
    }

    if (channels.size() > 1)
    {
      qWarning() << "Tiling not supported by Stereological Inclusion Extension";
    }
    else if (channels.size() == 1)
    {
      auto channel        = channels.first();
      auto edgesExtension = retrieveOrCreateStackExtension<ChannelEdges>(channel, m_factory);
      auto spacing        = channel->output()->spacing();
      const NmVector3 DELTA{ 0.5 * spacing[0], 0.5 * spacing[1], 0.5 * spacing[2] };

      Nm distances[6];
      if (edgesExtension->useDistanceToBounds())
      {
        edgesExtension->distanceToBounds(m_extendedItem, distances);
      }
      else
      {
        edgesExtension->distanceToEdges(m_extendedItem, distances);
      }

      for(int i = 0; i < 3; ++i)
      {
        isOnEdge |= (distances[2*i]     < DELTA[i]);
        isOnEdge |= (distances[(2*i)+1] < DELTA[i]);
      }
    }
  }

  updateInfoCache(TOUCH_EDGES, isOnEdge ? 1 : 0);

  return isOnEdge;
}
