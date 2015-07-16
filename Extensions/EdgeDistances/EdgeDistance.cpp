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

// Qt
#include <QDebug>

using namespace ESPINA;

const SegmentationExtension::Type EdgeDistance::TYPE = "EdgeDistance";

const SegmentationExtension::Key EdgeDistance::LEFT_DISTANCE   = "Left Distance";
const SegmentationExtension::Key EdgeDistance::TOP_DISTANCE    = "Top Distance";
const SegmentationExtension::Key EdgeDistance::FRONT_DISTANCE  = "Front Distance";
const SegmentationExtension::Key EdgeDistance::RIGHT_DISTANCE  = "Right Distance";
const SegmentationExtension::Key EdgeDistance::BOTTOM_DISTANCE = "Bottom Distance";
const SegmentationExtension::Key EdgeDistance::BACK_DISTANCE   = "Back Distance";

//-----------------------------------------------------------------------------
EdgeDistance::EdgeDistance(const SegmentationExtension::InfoCache& cache, const State& state)
: SegmentationExtension(cache)
{
}

//-----------------------------------------------------------------------------
EdgeDistance::~EdgeDistance()
{
}

//-----------------------------------------------------------------------------
State EdgeDistance::state() const
{
  State state;

  return state;
}

//-----------------------------------------------------------------------------
Snapshot EdgeDistance::snapshot() const
{
  Snapshot snapshot;

  return snapshot;
}

//-----------------------------------------------------------------------------
SegmentationExtension::KeyList EdgeDistance::availableInformation() const
{
  KeyList tags;

  tags << LEFT_DISTANCE;
  tags << RIGHT_DISTANCE;
  tags << TOP_DISTANCE;
  tags << BOTTOM_DISTANCE;
  tags << FRONT_DISTANCE;
  tags << BACK_DISTANCE;

  return tags;
}

//------------------------------------------------------------------------
void EdgeDistance::onExtendedItemSet(Segmentation* segmentation)
{
}

//------------------------------------------------------------------------
QVariant EdgeDistance::cacheFail(const QString& tag) const
{
  updateDistances();

  return cachedInfo(tag);
}

//-----------------------------------------------------------------------------
void EdgeDistance::edgeDistance(Nm distances[6]) const
{
  distances[0] = information(LEFT_DISTANCE).toDouble();
  distances[1] = information(RIGHT_DISTANCE).toDouble();
  distances[2] = information(TOP_DISTANCE).toDouble();
  distances[3] = information(BOTTOM_DISTANCE).toDouble();
  distances[4] = information(FRONT_DISTANCE).toDouble();
  distances[5] = information(BACK_DISTANCE).toDouble();
}

//-----------------------------------------------------------------------------
void EdgeDistance::updateDistances() const
{
  //qDebug() << "Updating" << m_seg->data().toString() << EdgeDistanceID;
  // Preven updating if all available information is already computed
  if (readyInformation().size() < 6)
  {
    QMutexLocker lock(&m_mutex);

    auto channels = QueryRelations::channels(m_extendedItem);

    if (channels.size() == 1)
    {
      Nm distances[6];
      auto channel = channels.first();

      auto edgesExtension = retrieveOrCreateExtension<ChannelEdges>(channel->extensions());

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
  }
}

//-----------------------------------------------------------------------------
EdgeDistancePtr ESPINA::edgeDistance(SegmentationExtensionPtr extension)
{
  return dynamic_cast<EdgeDistancePtr>(extension);
}
