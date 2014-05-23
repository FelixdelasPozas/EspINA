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
#include "CountingFrames/OrtogonalCountingFrame.h"
#include <CountingFrames/AdaptiveCountingFrame.h>
#include <CountingFrameManager.h>
#include <Core/Analysis/Segmentation.h>
#include <Core/Analysis/Query.h>
#include <Core/Analysis/Channel.h>
#include <Extensions/ExtensionUtils.h>

#include <QDebug>
#include <QApplication>

using namespace EspINA;
using namespace EspINA::CF;

ChannelExtension::Type CountingFrameExtension::TYPE = "CountingFrame";

const QString CountingFrameExtension::FILE = CountingFrameExtension::TYPE + "/CountingFrame.ext";

const std::string FILE_VERSION = CountingFrameExtension::TYPE.toStdString() + " 2.0\n";

const char SEP = ';';

//-----------------------------------------------------------------------------
CountingFrameExtension::CountingFrameExtension(CountingFrameManager* manager,
                                               SchedulerSPtr         scheduler,
                                               const State          &state)
: ChannelExtension(InfoCache())
, m_manager(manager)
, m_scheduler(scheduler)
, m_prevState(state)
{
}

//-----------------------------------------------------------------------------
CountingFrameExtension::~CountingFrameExtension()
{
  //qDebug() << "Deleting Counting Frame Channel Extension";
  for (auto cf : m_countingFrames)
  {
    m_manager->unregisterCountingFrame(cf);
  }
}

//-----------------------------------------------------------------------------
State CountingFrameExtension::state() const
{
  State state;

  QString br =  "";
  for(auto cf : m_countingFrames)
  {
    Nm inclusion[3], exclusion[3];
    cf->margins(inclusion, exclusion);
    // Id,Type,Constraint,Left, Top, Front, Right, Bottom, Back
    state += QString("%1%2,%3,%4,%5,%6,%7,%8,%9,%10").arg(br)
                                                 .arg(cf->id())
                                                 .arg(cf->cfType())
                                                 .arg(cf->categoryConstraint())
                                                 .arg(inclusion[0])
                                                 .arg(inclusion[1])
                                                 .arg(inclusion[2])
                                                 .arg(exclusion[0])
                                                 .arg(exclusion[1])
                                                 .arg(exclusion[2]);
    br = '\n';
  }

  return state;
}

//-----------------------------------------------------------------------------
Snapshot CountingFrameExtension::snapshot() const
{
  return Snapshot();
}

//-----------------------------------------------------------------------------
void CountingFrameExtension::createCountingFrame(CFType type,
                                                 Nm inclusion[3],
                                                 Nm exclusion[3],
                                                 const QString& constraint)
{
  createCountingFrame(type,
                      m_manager->defaultCountingFrameId(constraint),
                      inclusion,
                      exclusion,
                      constraint);
}

//-----------------------------------------------------------------------------
void CountingFrameExtension::deleteCountingFrame(CountingFrame* countingFrame)
{
  Q_ASSERT(m_countingFrames.contains(countingFrame));
  for (auto segmentation : QueryContents::segmentationsOnChannelSample(m_extendedItem))
  {
    auto extension = retrieveOrCreateExtension<StereologicalInclusion>(segmentation);
    extension->removeCountingFrame(countingFrame);
  }

  m_countingFrames.removeOne(countingFrame);

  m_manager->unregisterCountingFrame(countingFrame);

  if (m_countingFrames.isEmpty())
  {
    m_extendedItem->deleteExtension(TYPE);
  }

  delete countingFrame;
}

//-----------------------------------------------------------------------------
void CountingFrameExtension::onExtendedItemSet(Channel *channel)
{
  const int ID_POS              = 0;
  const int TYPE_POS            = 1;
  const int CONSTRAINT_POS      = 2;
  const int INCLUSION_START_POS = 3;
  const int EXCLUSION_START_POS = 6;
  const int NUM_FIELDS          = 9;

  if (!m_prevState.isEmpty())
  {
    for (auto cfEntry : m_prevState.split("\n"))
    {

      auto params = cfEntry.split(",");

      if (params.size() % NUM_FIELDS != 0)
      {
        qWarning() << "Invalid CF Extension state:\n" << m_prevState;
      } else
      {
        CFType type = static_cast<CFType>(params[TYPE_POS].toInt());

        Nm inclusion[3];
        Nm exclusion[3];

        for (int i = 0; i < 3; ++i)
        {
          inclusion[i] = params[INCLUSION_START_POS + i].toDouble();
          exclusion[i] = params[EXCLUSION_START_POS + i].toDouble();
        }

        createCountingFrame(type, params[ID_POS], inclusion, exclusion, params[CONSTRAINT_POS]);
      }
    }
  }
}

//-----------------------------------------------------------------------------
void CountingFrameExtension::onCountingFrameUpdated(CountingFrame* countingFrame)
{
  for(auto segmentation : QueryContents::segmentationsOnChannelSample(m_extendedItem))
  {
    auto extension = retrieveOrCreateExtension<StereologicalInclusion>(segmentation);
    extension->evaluateCountingFrame(countingFrame);
  }
}

//-----------------------------------------------------------------------------
void CountingFrameExtension::createCountingFrame(CFType type,
                                                 CountingFrame::Id id,
                                                 Nm inclusion[3],
                                                 Nm exclusion[3],
                                                 const QString& constraint)
{
  CountingFrame *cf;

  if (CFType::ORTOGONAL == type)
  {
    cf = OrtogonalCountingFrame::New(this, m_extendedItem->bounds(), inclusion, exclusion, m_scheduler);
  } else if (CFType::ADAPTIVE == type)
  {
    cf = AdaptiveCountingFrame::New(this, m_extendedItem->bounds(), inclusion, exclusion, m_scheduler);
  } else
  {
    Q_ASSERT(false);
  }

  cf->setId(id);
  cf->setCategoryConstraint(constraint);

  m_countingFrames << cf;

  m_manager->registerCountingFrame(cf);
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
CountingFrameExtensionSPtr EspINA::CF::countingFrameExtensionPtr(ChannelExtensionSPtr extension)
{
  return std::dynamic_pointer_cast<CountingFrameExtension>(extension);
}
