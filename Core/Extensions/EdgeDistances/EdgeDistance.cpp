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

QMap<SegmentationPtr, EdgeDistance::CacheEntry> EdgeDistance::s_cache;

const ModelItem::ExtId EdgeDistance::ID = "EdgeDistance";

const std::string FILE_VERSION = EdgeDistance::ID.toStdString() + " 1.0\n";
const char SEP = ',';

const Segmentation::InfoTag EdgeDistance::LEFT_DISTANCE   = "Left Distance";
const Segmentation::InfoTag EdgeDistance::TOP_DISTANCE    = "Top Distance";
const Segmentation::InfoTag EdgeDistance::UPPER_DISTANCE  = "Upper Distance";
const Segmentation::InfoTag EdgeDistance::RIGHT_DISTANCE  = "Right Distance";
const Segmentation::InfoTag EdgeDistance::BOTTOM_DISTANCE = "Bottom Distance";
const Segmentation::InfoTag EdgeDistance::LOWER_DISTANCE  = "Lower Distance";

//-----------------------------------------------------------------------------
EdgeDistance::CacheEntry::CacheEntry()
{
  for(int i=0; i<3; i++)
    Distances[i] = -1;
}

//-----------------------------------------------------------------------------
EdgeDistance::EdgeDistance()
{
  memset(m_distances, 0, 6*sizeof(double));
}

//-----------------------------------------------------------------------------
EdgeDistance::~EdgeDistance()
{
  qDebug() << "Deleting EdgeDistance Extension";
}

//-----------------------------------------------------------------------------
ModelItem::ExtId EdgeDistance::id()
{
  return ID;
}

//-----------------------------------------------------------------------------
void EdgeDistance::initialize(ModelItem::Arguments args)
{
  qDebug() << "Initialize (empty)" << m_seg->data().toString() << ID;
}

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
  bool cached = s_cache.contains(m_seg);
  if (!cached)
  {
    updateDistances();
  }

  if (LEFT_DISTANCE == tag)
    return s_cache[m_seg].Distances[0];
  if (RIGHT_DISTANCE == tag)
    return s_cache[m_seg].Distances[1];
  if (TOP_DISTANCE == tag)
    return s_cache[m_seg].Distances[2];
  if (BOTTOM_DISTANCE == tag)
    return s_cache[m_seg].Distances[3];
  if (UPPER_DISTANCE == tag)
    return s_cache[m_seg].Distances[4];
  if (LOWER_DISTANCE == tag)
    return s_cache[m_seg].Distances[5];

  qWarning() << ID << ":"  << tag << " is not provided";
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
    //TODO: Remove extensions when removed inside undo commands
    //Q_ASSERT(extensionSegmentation);

    if (extensionSegmentation)
    {
      for(i=0; i<6; i++)
        s_cache[extensionSegmentation].Distances[i] = fields[2+i].toDouble();
    }
  }

  return true;
}

//-----------------------------------------------------------------------------
bool EdgeDistance::saveCache(CacheList &cacheList)
{
  if (s_cache.isEmpty())
    return false;

  std::ostringstream cache;
  cache << FILE_VERSION;

  SegmentationPtr segmentation;
  foreach(segmentation, s_cache.keys())
  {
    cache << segmentation->filter()->id().toStdString();
    cache << SEP << segmentation->outputId();

    for(int i=0; i<6; i++)
      cache << SEP << s_cache[segmentation].Distances[i];

    cache << std::endl;
  }

  cacheList << QPair<QString, QByteArray>(EXTENSION_FILE, cache.str().c_str());

  return true;
}

//-----------------------------------------------------------------------------
Segmentation::Information * EdgeDistance::clone()
{
  return new EdgeDistance();
}

//-----------------------------------------------------------------------------
void EdgeDistance::updateDistances() const
{
  qDebug() << "Updating" << m_seg->data().toString() << ID;
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
  memcpy(s_cache[m_seg].Distances, distances, 6*sizeof(Nm));
}

//-----------------------------------------------------------------------------
EdgeDistancePtr EspINA::edgeDistancePtr(ModelItem::ExtensionPtr extension)
{
  EdgeDistancePtr res;
  res = dynamic_cast<EdgeDistancePtr>(extension);
  Q_ASSERT(res);

  return res;
}
