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


#include "CountingFrameExtension.h"

#include "CountingFramePanel.h"
#include "StereologicalInclusion.h"
#include "CountingFrames/CountingFrame.h"
#include "CountingFrames/RectangularCountingFrame.h"
#include "CountingFrames/AdaptiveCountingFrame.h"

#include <Core/Model/Sample.h>
#include <Core/Model/Channel.h>
#include <Core/Extensions/EdgeDistances/AdaptiveEdges.h>
#include <Core/Extensions/EdgeDistances/EdgeDistance.h>
#include <Core/Relations.h>
#include <GUI/ViewManager.h>

#include <QDebug>
#include <QApplication>

using namespace EspINA;

const QString CountingFrameExtension::EXTENSION_FILE = CountingFrameExtensionID + "/CountingFrameExtension.ext";

const std::string FILE_VERSION = CountingFrameExtensionID.toStdString() + " 1.0\n";

const char SEP = ';';

CountingFrameExtension::ExtensionCache CountingFrameExtension::s_cache;

//-----------------------------------------------------------------------------
CountingFrameExtension::CountingFrameExtension(CountingFramePanel *plugin,
                                               ViewManager        *viewManager)
: m_plugin(plugin)
, m_viewManager(viewManager)
{
}

//-----------------------------------------------------------------------------
CountingFrameExtension::~CountingFrameExtension()
{
  //qDebug() << "Deleting Counting Frame Channel Extension";
  foreach(CountingFrame *cf, m_countingFrames)
    m_plugin->deleteCountingFrame(cf);
}

//-----------------------------------------------------------------------------
ModelItem::ExtIdList CountingFrameExtension::dependencies() const
{
  ModelItem::ExtIdList deps;
  deps << AdaptiveEdgesID;
  return deps;
}

//-----------------------------------------------------------------------------
void CountingFrameExtension::loadCache(QuaZipFile  &file,
                                       const QDir  &tmpDir,
                                       IEspinaModel *model)
{
  QString header(file.readLine());
  if (header.toStdString() == FILE_VERSION)
  {
    ChannelPtr extensionChannel = NULL;
    CountingFrameExtensionPtr cfExtension = NULL;

    char buffer[1024];
    while (file.readLine(buffer, sizeof(buffer)) > 0)
    {
      QString line(buffer);
      QStringList fields = line.split(SEP);

      if (fields.size() == 2)
      {
        int i = 0;
        while (!extensionChannel && i < model->channels().size())
        {
          ChannelSPtr channel = model->channels()[i];
          if ( channel->filter()->id()       == fields[0]
            && channel->outputId()           == fields[1].toInt()
            && channel->filter()->cacheDir() == tmpDir)
          {
            extensionChannel = channel.data();
          }
          i++;
        }
        if (extensionChannel)
        {
          cfExtension = new CountingFrameExtension(m_plugin, m_viewManager);
          extensionChannel->addExtension(cfExtension);
        } else
        {
          qWarning() << CountingFrameExtensionID << "Invalid Cache Entry:" << line;
        }
      }
      else if (fields.size() == 8)
      {
        CF cfInfo;
        CountingFrame::Id id = fields[0].toInt();

        cfInfo.Type = static_cast<CFType>(fields[1].toInt());
        for(int i=0; i<3; i++)
          cfInfo.Inclusion[i] = fields[2+i].toDouble();
        for(int i=0; i<3; i++)
          cfInfo.Exclusion[i] = fields[5+i].toDouble();

        ExtensionData &data = s_cache[extensionChannel].Data;
        data.insert(id, cfInfo);

        CountingFrame *cf = NULL;
        switch(data[id].Type)
        {
          case ADAPTIVE:
            cf = AdaptiveCountingFrame::New(id, cfExtension, cfInfo.Inclusion, cfInfo.Exclusion, m_viewManager);
            break;
          case RECTANGULAR:
            double borders[6];
            extensionChannel->volume()->bounds(borders);
            cf = RectangularCountingFrame::New(id, cfExtension, borders, cfInfo.Inclusion, cfInfo.Exclusion, m_viewManager);
            break;
        };
        if (cf)
        {
          m_plugin->registerCF(cfExtension, cf);
        } else
        {
          qWarning() << CountingFrameExtensionID << "Invalid Cache Entry:" << line;
        }
      }
      else
        Q_ASSERT(false);

    }
  }
}

// File Output:
// ID version
// Channel Identifiers
// List of counting frames definitions separated by endl
// Channel Identifiers
// List of counting frames definitions separated by endl
// ...
//-----------------------------------------------------------------------------
bool CountingFrameExtension::saveCache(Snapshot &cacheList)
{
  s_cache.purge();

  if (s_cache.isEmpty())
    return false;

  std::ostringstream cache;
  cache << FILE_VERSION;

  ChannelPtr channel;
  foreach(channel, s_cache.keys())
  {
    cache << channel->filter()->id().toStdString();
    cache << SEP << channel->outputId();
    cache << std::endl;

    ExtensionData &data = s_cache[channel].Data;

    ExtensionData::iterator cf = data.begin();
    while(cf != data.end())
    {
      cache << cf.key();
      cache << SEP << cf->Type;

      for(int i=0; i<3; i++)
        cache << SEP << cf->Inclusion[i];

      for(int i=0; i<3; i++)
        cache << SEP << cf->Exclusion[i];

      cache << std::endl;
      ++cf;
    }
  }

  cacheList << SnapshotEntry(EXTENSION_FILE, cache.str().c_str());

  return true;
}

//-----------------------------------------------------------------------------
Channel::ExtensionPtr CountingFrameExtension::clone()
{
  return new CountingFrameExtension(m_plugin, m_viewManager);
}

//-----------------------------------------------------------------------------
void CountingFrameExtension::addCountingFrame(CountingFrame* countingFrame)
{
  Q_ASSERT(!m_countingFrames.contains(countingFrame));
  m_countingFrames << countingFrame;

  CF cf;
  if (countingFrame->name() == ADAPTIVE_CF)
    cf.Type = ADAPTIVE;
  else if (countingFrame->name() == RECTANGULAR_CF)
    cf.Type = RECTANGULAR;
  else
    Q_ASSERT(false);

  countingFrame->margins(cf.Inclusion, cf.Exclusion);

  s_cache[m_channel].Data.insert(countingFrame->id() ,cf);
  s_cache.markAsClean(m_channel);

  SampleSPtr sample = m_channel->sample();
  foreach(SegmentationPtr segmentation, sample->segmentations())
  {
    StereologicalInclusion *inclusionExtension = stereologicalInclusion(segmentation);
    inclusionExtension->setCountingFrames(m_countingFrames);
  }
  connect(countingFrame, SIGNAL(modified(CountingFrame*)),
          this, SLOT(countinfFrameUpdated(CountingFrame*)));
}

//-----------------------------------------------------------------------------
void CountingFrameExtension::deleteCountingFrame(CountingFrame* countingFrame)
{
  Q_ASSERT(m_countingFrames.contains(countingFrame));
  m_countingFrames.removeOne(countingFrame);

  SampleSPtr sample = m_channel->sample();
  foreach(SegmentationPtr segmentation, sample->segmentations())
  {
    StereologicalInclusion *inclusionExtension = stereologicalInclusion(segmentation);
    inclusionExtension->setCountingFrames(m_countingFrames);
  }
}

//-----------------------------------------------------------------------------
void CountingFrameExtension::initialize()
{

}

//-----------------------------------------------------------------------------
void CountingFrameExtension::invalidate(ChannelPtr channel)
{

}


//-----------------------------------------------------------------------------
void CountingFrameExtension::countinfFrameUpdated(CountingFrame* countingFrame)
{
  SampleSPtr sample = m_channel->sample();
  foreach(SegmentationPtr segmentation, sample->segmentations())
  {
    StereologicalInclusion *inclusionExtension = stereologicalInclusion(segmentation);
    inclusionExtension->evaluateCountingFrame(countingFrame);
  }
}

//-----------------------------------------------------------------------------
StereologicalInclusion *CountingFrameExtension::stereologicalInclusion(SegmentationPtr segmentation)
{
  StereologicalInclusion *inclusionExtension   = NULL;
  Segmentation::InformationExtension extension = segmentation->informationExtension(StereologicalInclusionID);
  if (extension)
  {
    inclusionExtension = dynamic_cast<StereologicalInclusion *>(extension);
  }
  else
  {
    inclusionExtension = new StereologicalInclusion();
    segmentation->addExtension(inclusionExtension);
  }
  Q_ASSERT(inclusionExtension);

  return inclusionExtension;
}

//-----------------------------------------------------------------------------
CountingFrameExtensionPtr EspINA::countingFrameExtensionPtr(Channel::ExtensionPtr extension)
{
  return dynamic_cast<CountingFrameExtensionPtr>(extension);
}
