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
  qDebug() << "Deleting Counting Frame Channel Extension";
  foreach(CountingFrame *cf, m_countingFrames)
    m_plugin->deleteCountingFrame(cf);
}

// //-----------------------------------------------------------------------------
// void CountingFrameExtension::initialize(ModelItem::Arguments args)
// {
//   qDebug() << "Initialize" << m_channel->data().toString() << CountingFrameExtensionID;
//   ModelItem::Arguments extArgs(args.value(CountingFrameExtensionID, QString()));
//   QStringList countingFrames;
// 
//   if (extArgs.isEmpty())
//   {
//     extArgs = ModelItem::Arguments(args.value(ID_1_2_5, QString()));
//     countingFrames = extArgs.value(COUNTING_FRAMES_1_2_5, "").split(";", QString::SkipEmptyParts);
//   }
//   else
//   {
//     countingFrames = extArgs.value(COUNTING_FRAMES, "").split(";", QString::SkipEmptyParts);
//   }
// 
//   foreach (QString countingFrame, countingFrames)
//   {
//     if (countingFrame.isEmpty())
//       continue;
// 
//     QString type = countingFrame.section('=',0,0);
//     QStringList margins = countingFrame.section('=',-1).split(',');
//     Nm inclusion[3], exclusion[3];
//     for (int i=0; i<3; i++)
//     {
//       inclusion[i] = margins[i].toDouble();
//       exclusion[i] = margins[3+i].toDouble();
//     }
//     if (RectangularCountingFrame::ID == type)
//       m_plugin->createRectangularCF(m_channel, inclusion, exclusion);
//     else if (AdaptiveCountingFrame::ID == type)
//       m_plugin->createAdaptiveCF(m_channel, inclusion, exclusion);
//   }
// }

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
                                       EspinaModel *model)
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
        CF cf;
        CountingFrame::Id id = fields[0].toInt();

        cf.Type = static_cast<CFType>(fields[1].toInt());
        for(int i=0; i<3; i++)
          cf.Inclusion[i] = fields[2+i].toDouble();
        for(int i=0; i<3; i++)
          cf.Exclusion[i] = fields[5+i].toDouble();

        ExtensionData &data = s_cache[extensionChannel].Data;
        data.insert(id, cf);

        switch(data[id].Type)
        {
          case ADAPTIVE:
            cfExtension->addCountingFrame(AdaptiveCountingFrame::New(id, cfExtension, cf.Inclusion, cf.Exclusion, m_viewManager));
            break;
          case RECTANGULAR:
            double borders[6];
            extensionChannel->volume()->bounds(borders);
            cfExtension->addCountingFrame(RectangularCountingFrame::New(id, cfExtension, borders, cf.Inclusion, cf.Exclusion, m_viewManager));
            break;
        };
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
  return false;
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
  cf.Type = static_cast<CFType>(countingFrame->type());
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
  Segmentation::InformationExtension extension = segmentation->informationExtension(StereologicalInclusion::ID);
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
