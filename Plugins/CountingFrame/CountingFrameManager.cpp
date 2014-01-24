/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2013  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "CountingFrameManager.h"
#include "Extensions/CountingFrameExtension.h"
#include "CountingFrames/RectangularCountingFrame.h"
#include "CountingFrames/AdaptiveCountingFrame.h"
#include <Core/Analysis/Channel.h>

using namespace EspINA;
using namespace EspINA::CF;

//------------------------------------------------------------------------
void CountingFrameManager::createAdaptiveCF(ChannelAdapterPtr channel,
                                            Nm inclusion[3],
                                            Nm exclusion[3])
{
  int id = nextId();

  auto extension = retrieveOrCreateCFExtension(channel);

  auto cf = AdaptiveCountingFrame::New(id, extension, channel->bounds(), inclusion, exclusion);

  registerCountingFrame(cf, extension);
}

//------------------------------------------------------------------------
void CountingFrameManager::createRectangularCF(ChannelAdapterPtr channel,
                                               Nm inclusion[3],
                                               Nm exclusion[3])
{
  int id = nextId();

  auto extension = retrieveOrCreateCFExtension(channel);

  auto cf = RectangularCountingFrame::New(id, extension, channel->bounds(), inclusion, exclusion);

  registerCountingFrame(cf, extension);
}

//-----------------------------------------------------------------------------
void CountingFrameManager::deleteCountingFrame(CountingFrame* cf)
{
  Q_ASSERT(m_countingFrames.contains(cf));

  auto channel   = m_countingFrames[cf];

  Q_ASSERT(channel->hasExtension(CountingFrameExtension::TYPE));

  auto extension   = channel->extension(CountingFrameExtension::TYPE);
  auto cfExtension = std::dynamic_pointer_cast<CountingFrameExtension>(extension);

  cfExtension->removeCountingFrame(cf);

  if (cfExtension->countingFrames().isEmpty())
  {
    channel->deleteExtension(cfExtension);
  }


//   while (!cfExtension && i < m_countingFramesExtensions.size())
//   {
//     if (m_countingFramesExtensions[i]->countingFrames().contains(cf))
//     {
//       cfExtension = m_countingFramesExtensions[i];
//       cfExtension->deleteCountingFrame(cf);
//     }
//     else
//       ++i;
//   }
//   m_countingFramesExtensions.removeAt(i);
//   m_countingFrames.removeOne(cf);

  m_countingFrames.remove(cf);
  Q_ASSERT(!m_countingFrames.contains(cf));

  cf->Delete();
}

//-----------------------------------------------------------------------------
CountingFrameExtension* CountingFrameManager::retrieveOrCreateCFExtension(ChannelAdapterPtr channel)
{
  CountingFrameExtension *extension = nullptr;

  if (channel->hasExtension(CountingFrameExtension::TYPE))
  {
    extension = dynamic_cast<CountingFrameExtension *>(channel->extension(CountingFrameExtension::TYPE).get());
  }
  else
  {
    extension =  new CountingFrameExtension(this);
    channel->addExtension(CountingFrameExtensionSPtr{extension});
  }
  Q_ASSERT(extension);

  return extension;
}


//-----------------------------------------------------------------------------
void CountingFrameManager::registerCountingFrame(CountingFrame* cf, CountingFrameExtension* extension)
{
  extension->addCountingFrame(cf);

  m_countingFrames[cf] = extension->channel();

  emit countingFrameCreated(cf);
}

//-----------------------------------------------------------------------------
int CountingFrameManager::nextId() const
{
  int lastId = -1;

  for (auto cf :m_countingFrames.keys())
  {
    lastId = std::max(cf->id(), lastId);
  }

  return lastId + 1;
}

