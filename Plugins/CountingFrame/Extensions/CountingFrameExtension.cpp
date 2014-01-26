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

#include "CountingFrames/CountingFrame.h"
#include "CountingFrames/RectangularCountingFrame.h"
#include <CountingFrames/AdaptiveCountingFrame.h>
#include <Core/Analysis/Segmentation.h>
#include <Core/Analysis/Query.h>
#include <Core/Analysis/Channel.h>

#include <QDebug>
#include <QApplication>

using namespace EspINA;
using namespace EspINA::CF;

ChannelExtension::Type CountingFrameExtension::TYPE = "CountingFrame";

const QString CountingFrameExtension::FILE = CountingFrameExtension::TYPE + "/CountingFrame.ext";

const std::string FILE_VERSION = CountingFrameExtension::TYPE.toStdString() + " 2.0\n";

const char SEP = ';';

//-----------------------------------------------------------------------------
CountingFrameExtension::CountingFrameExtension(CountingFrameManager* manager, State state)
: m_manager(manager)
, m_prevState(state)
{
}

//-----------------------------------------------------------------------------
CountingFrameExtension::~CountingFrameExtension()
{
  //qDebug() << "Deleting Counting Frame Channel Extension";
}

//-----------------------------------------------------------------------------
//TODO ModelItem::ExtIdList CountingFrameExtension::dependencies() const
// {
//   ModelItem::ExtIdList deps;
//   deps << AdaptiveEdgesID;
//   return deps;
// }

// //-----------------------------------------------------------------------------
// void CountingFrameExtension::loadCache(QuaZipFile  &file,
//                                        const QDir  &tmpDir,
//                                        IEspinaModel *model)
// {
//   QString header(file.readLine());
//   if (header.toStdString() == FILE_VERSION)
//   {
//     ChannelPtr extensionChannel = NULL;
//     CountingFrameExtensionPtr cfExtension = NULL;
//
//     char buffer[1024];
//     while (file.readLine(buffer, sizeof(buffer)) > 0)
//     {
//       QString line(buffer);
//       QStringList fields = line.split(SEP);
//
//       if (fields.size() == 2)
//       {
//         int i = 0;
//         while (!extensionChannel && i < model->channels().size())
//         {
//           ChannelSPtr channel = model->channels()[i];
//           if ( channel->filter()->id()       == fields[0]
//             && channel->outputId()           == fields[1].toInt()
//             && channel->filter()->cacheDir() == tmpDir)
//           {
//             extensionChannel = channel.get();
//           }
//           i++;
//         }
//         if (extensionChannel)
//         {
//           cfExtension = new CountingFrameExtension(m_plugin, m_viewManager);
//           extensionChannel->addExtension(cfExtension);
//         } else
//         {
//           qWarning() << CountingFrameExtensionID << "Invalid Cache Entry:" << line;
//         }
//       }
//       else if (fields.size() == 8)
//       {
//         CF cfInfo;
//         CountingFrame::Id id = fields[0].toInt();
//
//         cfInfo.Type = static_cast<CFType>(fields[1].toInt());
//         for(int i=0; i<3; i++)
//           cfInfo.Inclusion[i] = fields[2+i].toDouble();
//         for(int i=0; i<3; i++)
//           cfInfo.Exclusion[i] = fields[5+i].toDouble();
//
//         ExtensionData &data = s_cache[extensionChannel].Data;
//         data.insert(id, cfInfo);
//
//         CountingFrame *cf = NULL;
//         switch(data[id].Type)
//         {
//           case ADAPTIVE:
//             cf = AdaptiveCountingFrame::New(id, cfExtension, cfInfo.Inclusion, cfInfo.Exclusion, m_viewManager);
//             break;
//           case RECTANGULAR:
//             double borders[6];
//             extensionChannel->volume()->bounds(borders);
//             cf = RectangularCountingFrame::New(id, cfExtension, borders, cfInfo.Inclusion, cfInfo.Exclusion, m_viewManager);
//             break;
//         };
//         if (cf)
//         {
//           m_plugin->registerCF(cfExtension, cf);
//         } else
//         {
//           qWarning() << CountingFrameExtensionID << "Invalid Cache Entry:" << line;
//         }
//       }
//       else
//         Q_ASSERT(false);
//
//     }
//   }
// }

// File Output:
// ID version
// Channel Identifiers
// List of counting frames definitions separated by endl
// Channel Identifiers
// List of counting frames definitions separated by endl
// ...
//-----------------------------------------------------------------------------
// bool CountingFrameExtension::saveCache(Snapshot &cacheList)
// {
//   s_cache.purge();
//
//   if (s_cache.isEmpty())
//     return false;
//
//   std::ostringstream cache;
//   cache << FILE_VERSION;
//
//   ChannelPtr channel;
//   foreach(channel, s_cache.keys())
//   {
//     cache << channel->filter()->id().toStdString();
//     cache << SEP << channel->outputId();
//     cache << std::endl;
//
//     ExtensionData &data = s_cache[channel].Data;
//
//     ExtensionData::iterator cf = data.begin();
//     while(cf != data.end())
//     {
//       cache << cf.key();
//       cache << SEP << cf->Type;
//
//       for(int i=0; i<3; i++)
//         cache << SEP << cf->Inclusion[i];
//
//       for(int i=0; i<3; i++)
//         cache << SEP << cf->Exclusion[i];
//
//       cache << std::endl;
//       ++cf;
//     }
//   }
//
//   cacheList << SnapshotEntry(FILE, cache.str().c_str());
//
//   return true;
// }
//-----------------------------------------------------------------------------
State CountingFrameExtension::state() const
{
  State state;

  QString bl =  "";
  for(auto cf : m_countingFrames)
  {
    Nm inclusion[3], exclusion[3];
    cf->margins(inclusion, exclusion);
    // Type, Left, Top, Front, Right, Bottom, Back
    state += QString("%1%2,%3,%4,%5,%6,%7,%8").arg(bl)
                                              .arg(cf->cfType())
                                              .arg(inclusion[0])
                                              .arg(inclusion[1])
                                              .arg(inclusion[2])
                                              .arg(exclusion[0])
                                              .arg(exclusion[1])
                                              .arg(exclusion[2]);
    bl = '\n';
  }

  return state;
}

//-----------------------------------------------------------------------------
Snapshot CountingFrameExtension::snapshot() const
{
  return Snapshot();
}

//-----------------------------------------------------------------------------
void CountingFrameExtension::addCountingFrame(CountingFrame* countingFrame)
{
  Q_ASSERT(!m_countingFrames.contains(countingFrame));
  m_countingFrames << countingFrame;

  for (auto segmentation : Query::segmentationsOnChannelSample(m_channel))
  {
    auto extension = stereologicalInclusionExtension(segmentation);
    extension->addCountingFrame(countingFrame);
  }

  connect(countingFrame, SIGNAL(modified(CountingFrame*)),
          this, SLOT(onCountingFrameUpdated(CountingFrame*)));
}

//-----------------------------------------------------------------------------
void CountingFrameExtension::removeCountingFrame(CountingFrame* countingFrame)
{
  Q_ASSERT(m_countingFrames.contains(countingFrame));
  m_countingFrames.removeOne(countingFrame);

  for (auto segmentation : Query::segmentationsOnChannelSample(m_channel))
  {
    auto extension = stereologicalInclusionExtension(segmentation);
    extension->removeCountingFrame(countingFrame);
  }
}

//-----------------------------------------------------------------------------
void CountingFrameExtension::onChannelSet(ChannelPtr channel)
{
  if (!m_prevState.isEmpty())
  {

    for (auto cfEntry : m_prevState.split("\n"))
    {
      const int numberOfFields = 7;

      auto params = cfEntry.split(",");

      if (params.size() % numberOfFields != 0)
      {
        qWarning() << "Invalid CF Extension state:\n" << m_prevState;
      } else
      {
        CFType type = params[0].toInt();

        Nm inclusion[3];
        Nm exclusion[3];

        for (int i = 0; i < 3; ++i)
        {
          inclusion[i] = params[i+1].toDouble();
          exclusion[i] = params[i+4].toDouble();
        }

        CountingFrame *cf;
        if (CFType::RECTANGULAR == type)
        {
          cf = AdaptiveCountingFrame::New(this, channel->bounds(), inclusion, exclusion);

        } else if (CFType::ADAPTIVE == type)
        {
          cf = RectangularCountingFrame::New(this, channel->bounds(), inclusion, exclusion);
        } else
        {
          Q_ASSERT(false);
        }

        m_manager->registerCountingFrame(cf, this);
      }
    }
  }
}

//-----------------------------------------------------------------------------
void CountingFrameExtension::onCountingFrameUpdated(CountingFrame* countingFrame)
{
  for(auto segmentation : Query::segmentationsOnChannelSample(m_channel))
  {
    auto extension = stereologicalInclusionExtension(segmentation);
    extension->evaluateCountingFrame(countingFrame);
  }
}

//-----------------------------------------------------------------------------
StereologicalInclusionSPtr CountingFrameExtension::stereologicalInclusionExtension(SegmentationSPtr segmentation)
{
  StereologicalInclusionSPtr stereologicalExtension;
  if (segmentation->hasExtension(StereologicalInclusion::TYPE))
  {
    auto extension = segmentation->extension(StereologicalInclusion::TYPE);
    stereologicalExtension = stereologicalInclusion(extension);
  }
  else
  {
    stereologicalExtension = StereologicalInclusionSPtr(new StereologicalInclusion());
    segmentation->addExtension(stereologicalExtension);
  }
  Q_ASSERT(stereologicalExtension);

  return stereologicalExtension;
}

// //-----------------------------------------------------------------------------
// CountingFrameExtensionPtr EspINA::CF::countingFrameExtensionPtr(ChannelExtensionSPtr extension)
// {
//   return dynamic_cast<CountingFrameExtensionPtr>(extension);
// }
