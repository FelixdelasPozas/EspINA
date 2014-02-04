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


#include "EdgeDistance.h"

#include "ChannelEdges.h"
#include <Core/Analysis/Channel.h>
#include <Core/Analysis/Segmentation.h>
#include <Core/Analysis/Query.h>

#include <QDebug>

using namespace EspINA;

const SegmentationExtension::Type EdgeDistance::TYPE = "EdgeDistance";

const SegmentationExtension::InfoTag EdgeDistance::LEFT_DISTANCE   = "Left Distance";
const SegmentationExtension::InfoTag EdgeDistance::TOP_DISTANCE    = "Top Distance";
const SegmentationExtension::InfoTag EdgeDistance::FRONT_DISTANCE  = "Front Distance";
const SegmentationExtension::InfoTag EdgeDistance::RIGHT_DISTANCE  = "Right Distance";
const SegmentationExtension::InfoTag EdgeDistance::BOTTOM_DISTANCE = "Bottom Distance";
const SegmentationExtension::InfoTag EdgeDistance::BACK_DISTANCE   = "Back Distance";

//-----------------------------------------------------------------------------
EdgeDistance::EdgeDistance(const State& state, const SegmentationExtension::InfoCache& cache)
: SegmentationExtension(cache)
, m_init(false)
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
SegmentationExtension::InfoTagList EdgeDistance::availableInformations() const
{
  InfoTagList tags;

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

  updateInfoCache(LEFT_DISTANCE  , m_distances[0]);
  updateInfoCache(RIGHT_DISTANCE , m_distances[1]);
  updateInfoCache(TOP_DISTANCE   , m_distances[2]);
  updateInfoCache(BOTTOM_DISTANCE, m_distances[3]);
  updateInfoCache(FRONT_DISTANCE , m_distances[4]);
  updateInfoCache(BACK_DISTANCE  , m_distances[5]);

  if (LEFT_DISTANCE == tag)
    return m_distances[0];
  if (RIGHT_DISTANCE == tag)
    return m_distances[1];
  if (TOP_DISTANCE == tag)
    return m_distances[2];
  if (BOTTOM_DISTANCE == tag)
    return m_distances[3];
  if (FRONT_DISTANCE == tag)
    return m_distances[4];
  if (BACK_DISTANCE == tag)
    return m_distances[5];

  return QVariant();
}

//-----------------------------------------------------------------------------
// void EdgeDistance::loadCache(QuaZipFile  &file,
//                              const QDir  &tmpDir,
//                              IEspinaModel *model)
// {
//   QString header(file.readLine());
//   if (header.toStdString() == FILE_VERSION)
//   {
//     char buffer[1024];
//     while (file.readLine(buffer, sizeof(buffer)) > 0)
//     {
//       QString line(buffer);
//       QStringList fields = line.split(SEP);
// 
//       SegmentationPtr extensionSegmentation = NULL;
//       int i = 0;
//       while (!extensionSegmentation && i < model->segmentations().size())
//       {
//         SegmentationSPtr segmentation = model->segmentations()[i];
//         if ( segmentation->filter()->id()  == fields[0]
//           && segmentation->outputId()         == fields[1].toInt()
//           && segmentation->filter()->cacheDir() == tmpDir)
//         {
//           extensionSegmentation = segmentation.get();
//         }
//         i++;
//       }
//       if (extensionSegmentation)
//       {
//         ExtensionData &data = s_cache[extensionSegmentation].Data;
// 
//         for(i=0; i<6; i++)
//         {
//           data.Distances[i] = fields[2+i].toDouble();
//         }
//       } else
//       {
//         qWarning() << EdgeDistanceID << "Invalid Cache Entry:" << line;
//       }
//     }
//   }
// }


//-----------------------------------------------------------------------------
void EdgeDistance::edgeDistance(Nm distances[6]) const
{
  if (!m_init) updateDistances();

  memcpy(distances, m_distances, 6*sizeof(Nm));
}

//-----------------------------------------------------------------------------
void EdgeDistance::updateDistances() const
{
  //qDebug() << "Updating" << m_seg->data().toString() << EdgeDistanceID;
  auto channels = Query::channels(m_extendedItem);

  if (channels.size() == 1)
  {
    auto channel = channels.first();
    if (!channel->hasExtension(ChannelEdges::TYPE))
    {
      ChannelEdgesSPtr extension{new ChannelEdges()};
      channel->addExtension(extension);
    }
    ChannelEdgesPtr edgesExtension = channelEdgesExtension(channel->extension(ChannelEdges::TYPE).get());
    Q_ASSERT(edgesExtension);

    if (edgesExtension->useDistanceToBounds())
    {
      edgesExtension->distanceToBounds(m_extendedItem, m_distances);
    }
    else
    {
      edgesExtension->distanceToEdges(m_extendedItem, m_distances);
    }
  }

  m_init = true;
}

//-----------------------------------------------------------------------------
void EdgeDistance::setDistances(Nm distances[6])
{
  memcpy(m_distances, distances, 6*sizeof(Nm));
}

//-----------------------------------------------------------------------------
EdgeDistancePtr EspINA::edgeDistance(SegmentationExtensionPtr extension)
{
  return dynamic_cast<EdgeDistancePtr>(extension);
}
