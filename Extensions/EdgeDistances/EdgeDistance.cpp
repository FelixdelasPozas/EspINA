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

#include "AdaptiveEdges.h"
#include <Core/Analysis/Channel.h>
#include <Core/Analysis/Segmentation.h>

#include <QDebug>

using namespace EspINA;

const SegmentationExtension::Type EdgeDistance::TYPE = "EdgeDistance";
const QString EdgeDistance::EXTENSION_FILE = "EdgeDistances/EdgeDistance.csv";

const std::string FILE_VERSION = EdgeDistance::TYPE.toStdString() + " 1.0\n";
const char SEP = ',';

const SegmentationExtension::InfoTag EdgeDistance::LEFT_DISTANCE   = "Left Distance";
const SegmentationExtension::InfoTag EdgeDistance::TOP_DISTANCE    = "Top Distance";
const SegmentationExtension::InfoTag EdgeDistance::UPPER_DISTANCE  = "Upper Distance";
const SegmentationExtension::InfoTag EdgeDistance::RIGHT_DISTANCE  = "Right Distance";
const SegmentationExtension::InfoTag EdgeDistance::BOTTOM_DISTANCE = "Bottom Distance";
const SegmentationExtension::InfoTag EdgeDistance::LOWER_DISTANCE  = "Lower Distance";

//-----------------------------------------------------------------------------
EdgeDistance::EdgeDistance()
: m_init(false)
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
  tags << UPPER_DISTANCE;
  tags << LOWER_DISTANCE;

  return tags;
}

//------------------------------------------------------------------------
void EdgeDistance::onSegmentationSet(SegmentationPtr segmentation)
{
//   connect(segmentation->modified()),
//           this, SLOT(invalidate()));

  if (m_segmentation->isOutputModified())
    invalidate(); //TODO: Change to ignore storage data
}

//-----------------------------------------------------------------------------
QVariant EdgeDistance::information(const InfoTag &tag) const
{
  if (!m_init)
  {
    updateDistances();
  }

  if (LEFT_DISTANCE == tag)
    return m_distances[0];
  if (RIGHT_DISTANCE == tag)
    return m_distances[1];
  if (TOP_DISTANCE == tag)
    return m_distances[2];
  if (BOTTOM_DISTANCE == tag)
    return m_distances[3];
  if (UPPER_DISTANCE == tag)
    return m_distances[4];
  if (LOWER_DISTANCE == tag)
    return m_distances[5];

  qWarning() << TYPE << ":"  << tag << " is not provided";
  Q_ASSERT(false);
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
// bool EdgeDistance::saveCache(Snapshot &cacheList)
// {
//   s_cache.purge(invalidData);
// 
//   if (s_cache.isEmpty())
//     return false;
// 
//   std::ostringstream cache;
//   cache << FILE_VERSION;
// 
//   foreach(SegmentationPtr segmentation, s_cache.keys())
//   {
//     ExtensionData &data = s_cache[segmentation].Data;
// 
//     cache << segmentation->filter()->id().toStdString();
//     cache << SEP << segmentation->outputId();
// 
//     for(int i=0; i<6; i++)
//       cache << SEP << data.Distances[i];
// 
//     cache << std::endl;
//   }
// 
//   cacheList << SnapshotEntry(EXTENSION_FILE, cache.str().c_str());
// 
//   return true;
// }

//-----------------------------------------------------------------------------
void EdgeDistance::edgeDistance(Nm distances[6]) const
{
  if (!m_init) updateDistances();

  memcpy(distances, m_distances, 6*sizeof(Nm));
}

//-----------------------------------------------------------------------------
void EdgeDistance::invalidate()
{
}

//-----------------------------------------------------------------------------
void EdgeDistance::updateDistances() const
{
  //qDebug() << "Updating" << m_seg->data().toString() << EdgeDistanceID;
  ChannelSPtr channel; // TODO = m_segmentation->channel();
  if (channel)
  {
    if (!channel->hasExtension(AdaptiveEdges::TYPE))
    {
      AdaptiveEdgesSPtr extension{new AdaptiveEdges()};
      channel->addExtension(extension);
    }
    AdaptiveEdgesPtr edgesExtension = adaptiveEdges(channel->extension(AdaptiveEdges::TYPE).get());
    Q_ASSERT(edgesExtension);

    edgesExtension->computeDistanceToEdge(m_segmentation);
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
