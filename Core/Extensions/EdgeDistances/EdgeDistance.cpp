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
}

//-----------------------------------------------------------------------------
EdgeDistance::~EdgeDistance()
{
  if (m_segmentation)
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

//------------------------------------------------------------------------
void EdgeDistance::setSegmentation(SegmentationPtr seg)
{
  EspINA::Segmentation::Information::setSegmentation(seg);

  connect(m_segmentation, SIGNAL(outputModified()),
          this, SLOT(invalidate()));

  if (m_segmentation->outputIsModified())
    invalidate();
  else
    initialize();
}

//-----------------------------------------------------------------------------
QVariant EdgeDistance::information(const Segmentation::InfoTag &tag)
{
  bool cached = s_cache.isCached(m_segmentation);
  if (!cached)
  {
    updateDistances();
  }

  ExtensionData &data = s_cache[m_segmentation].Data;
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
void EdgeDistance::loadCache(QuaZipFile  &file,
                             const QDir  &tmpDir,
                             IEspinaModel *model)
{
  QString header(file.readLine());
  if (header.toStdString() == FILE_VERSION)
  {
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
  }
}

//-----------------------------------------------------------------------------
// It's declared static to avoid collisions with other functions with same
// signature in different compilation units
static bool invalidData(SegmentationPtr seg)
{
  return !seg->hasInformationExtension(EdgeDistanceID)
      && seg->outputIsModified();
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
void EdgeDistance::edgeDistance(Nm distances[6]) const
{
  if (!s_cache.isCached(m_segmentation))
      updateDistances();

  memcpy(distances, s_cache[m_segmentation].Data.Distances, 6*sizeof(Nm));
}

//-----------------------------------------------------------------------------
void EdgeDistance::initialize()
{
  if (m_segmentation)
  {
    s_cache.markAsClean(m_segmentation);
  }
}

//-----------------------------------------------------------------------------
void EdgeDistance::invalidate(SegmentationPtr segmentation)
{
  if (!segmentation)
    segmentation = m_segmentation;

  if (segmentation)
  {
    s_cache.markAsDirty(segmentation);
  }
}

//-----------------------------------------------------------------------------
void EdgeDistance::updateDistances() const
{
  //qDebug() << "Updating" << m_seg->data().toString() << EdgeDistanceID;
  ChannelSPtr channel = m_segmentation->channel();
  if (!channel.isNull())
  {
    AdaptiveEdges  *edgesExtension = NULL;
    Channel::ExtensionPtr extension = channel->extension(AdaptiveEdgesID);
    if (!extension)
    {
      edgesExtension = new AdaptiveEdges();
      channel->addExtension(edgesExtension);
    } else
    {
      edgesExtension = adaptiveEdgesPtr(extension);
    }
    Q_ASSERT(edgesExtension);

    edgesExtension->computeDistanceToEdge(m_segmentation);
  }
}

//-----------------------------------------------------------------------------
void EdgeDistance::setDistances(Nm distances[6])
{
  memcpy(s_cache[m_segmentation].Data.Distances, distances, 6*sizeof(Nm));
  s_cache.markAsClean(m_segmentation);
}

//-----------------------------------------------------------------------------
EdgeDistancePtr EspINA::edgeDistancePtr(ModelItem::ExtensionPtr extension)
{
  EdgeDistancePtr res;
  res = dynamic_cast<EdgeDistancePtr>(extension);
  Q_ASSERT(res);

  return res;
}
