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

#include "Core/Model/Segmentation.h"
#include "Core/Model/Channel.h"
#include <Core/Model/EspinaModel.h>

#include <QDebug>

using namespace EspINA;

const QString EdgeDistance::EXTENSION_FILE = "EdgeDistances/EdgeDistance.csv";

EdgeDistance::ExtensionCache EdgeDistance::s_cache;

const std::string FILE_VERSION = EdgeDistanceID.toStdString() + " 1.0\n";
const char SEP = ',';

const Segmentation::InfoTag EdgeDistance::LEFT_DISTANCE   = "Left Distance";
const Segmentation::InfoTag EdgeDistance::TOP_DISTANCE    = "Top Distance";
const Segmentation::InfoTag EdgeDistance::UPPER_DISTANCE  = "Upper Distance";
const Segmentation::InfoTag EdgeDistance::RIGHT_DISTANCE  = "Right Distance";
const Segmentation::InfoTag EdgeDistance::BOTTOM_DISTANCE = "Bottom Distance";
const Segmentation::InfoTag EdgeDistance::LOWER_DISTANCE  = "Lower Distance";

//-----------------------------------------------------------------------------
EdgeDistance::ExtensionData::ExtensionData()
{
  memset(Distances, 0, 6*sizeof(double));
}


//-----------------------------------------------------------------------------
EdgeDistance::EdgeDistance()
{
  memset(m_distances, 0, 6*sizeof(double));
}

//-----------------------------------------------------------------------------
EdgeDistance::~EdgeDistance()
{
  if (m_seg)
  {
    //qDebug() << m_seg->data().toString() << ": Deleting" << EdgeDistanceID;
    invalidate();
  }
}

//-----------------------------------------------------------------------------
ModelItem::ExtId EdgeDistance::id()
{
  return EdgeDistanceID;
}

//-----------------------------------------------------------------------------
void EdgeDistance::initialize(ModelItem::Arguments args)
{
  //qDebug() << "Initialize (empty)" << m_seg->data().toString() << EdgeDistanceID;
}

//-----------------------------------------------------------------------------
Segmentation::InfoTagList EdgeDistance::availableInformations() const
{
  Segmentation::InfoTagList tags;

  tags << LEFT_DISTANCE;
  tags << RIGHT_DISTANCE;
  tags << TOP_DISTANCE;
  tags << BOTTOM_DISTANCE;
  tags << UPPER_DISTANCE;
  tags << LOWER_DISTANCE;

  return tags;
}

//-----------------------------------------------------------------------------
QVariant EdgeDistance::information(const Segmentation::InfoTag &tag)
{
  bool cached = s_cache.isCached(m_seg);
  if (!cached)
  {
    updateDistances();
  }

  ExtensionData &data = s_cache[m_seg].Data;
  if (LEFT_DISTANCE == tag)
    return data.Distances[0];
  if (RIGHT_DISTANCE == tag)
    return data.Distances[1];
  if (TOP_DISTANCE == tag)
    return data.Distances[2];
  if (BOTTOM_DISTANCE == tag)
    return data.Distances[3];
  if (UPPER_DISTANCE == tag)
    return data.Distances[4];
  if (LOWER_DISTANCE == tag)
    return data.Distances[5];

  qWarning() << EdgeDistanceID << ":"  << tag << " is not provided";
  Q_ASSERT(false);
  return QVariant();
}


//-----------------------------------------------------------------------------
bool EdgeDistance::loadCache(QuaZipFile  &file,
                             const QDir  &tmpDir,
                             EspinaModel *model)
{
  QString header(file.readLine());
  if (header.toStdString() != FILE_VERSION)
    return false;

  char buffer[1024];
  while (file.readLine(buffer, sizeof(buffer)) > 0)
  {
    QString line(buffer);
    QStringList fields = line.split(SEP);

    SegmentationPtr extensionSegmentation = NULL;
    int i = 0;
    while (!extensionSegmentation && i < model->segmentations().size())
    {
      SegmentationSPtr segmentation = model->segmentations()[i];
      if ( segmentation->filter()->id()  == fields[0]
        && segmentation->outputId()         == fields[1].toInt()
        && segmentation->filter()->cacheDir() == tmpDir)
      {
        extensionSegmentation = segmentation.data();
      }
      i++;
    }
    if (extensionSegmentation)
    {
      ExtensionData &data = s_cache[extensionSegmentation].Data;

      for(i=0; i<6; i++)
      {
        data.Distances[i] = fields[2+i].toDouble();
      }
    } else
    {
      qWarning() << EdgeDistanceID << "Invalid Cache Entry:" << line;
    }
  }

  return true;
}

//------------------------------------------------------------------------
// It's declared static to avoid collisions with other functions with same
// signature in different compilation units
static bool invalidData(SegmentationPtr seg)
{
  return seg->informationExtension(EdgeDistanceID) == NULL
      && seg->isVolumeModified();
}

//-----------------------------------------------------------------------------
bool EdgeDistance::saveCache(Snapshot &cacheList)
{
  s_cache.purge(invalidData);

  if (s_cache.isEmpty())
    return false;

  std::ostringstream cache;
  cache << FILE_VERSION;

  foreach(SegmentationPtr segmentation, s_cache.keys())
  {
    ExtensionData &data = s_cache[segmentation].Data;

    cache << segmentation->filter()->id().toStdString();
    cache << SEP << segmentation->outputId();

    for(int i=0; i<6; i++)
      cache << SEP << data.Distances[i];

    cache << std::endl;
  }

  cacheList << SnapshotEntry(EXTENSION_FILE, cache.str().c_str());

  return true;
}

//-----------------------------------------------------------------------------
Segmentation::Information * EdgeDistance::clone()
{
  return new EdgeDistance();
}

//-----------------------------------------------------------------------------
void EdgeDistance::invalidate()
{
  if (m_seg)
  {
    //qDebug() << "Invalidate" << m_seg->data().toString() << EdgeDistanceID;
    s_cache.markAsDirty(m_seg);
  }

}

//-----------------------------------------------------------------------------
void EdgeDistance::updateDistances() const
{
  //qDebug() << "Updating" << m_seg->data().toString() << EdgeDistanceID;
  ChannelSPtr channel = m_seg->channel();

  AdaptiveEdges  *edgesExtension = NULL;
  Channel::ExtensionPtr extension = channel->extension(AdaptiveEdges::ID);
  if (!extension)
  {
    edgesExtension = new AdaptiveEdges();
    channel->addExtension(edgesExtension);
  } else
  {
    edgesExtension = adaptiveEdgesPtr(extension);
  }
  Q_ASSERT(edgesExtension);

  edgesExtension->computeDistanceToEdge(m_seg);
}

//-----------------------------------------------------------------------------
void EdgeDistance::setDistances(Nm distances[6])
{
  memcpy(s_cache[m_seg].Data.Distances, distances, 6*sizeof(Nm));
  s_cache.markAsClean(m_seg);
}

//-----------------------------------------------------------------------------
EdgeDistancePtr EspINA::edgeDistancePtr(ModelItem::ExtensionPtr extension)
{
  EdgeDistancePtr res;
  res = dynamic_cast<EdgeDistancePtr>(extension);
  Q_ASSERT(res);

  return res;
}
