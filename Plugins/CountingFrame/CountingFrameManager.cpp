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

using namespace EspINA;
using namespace EspINA::CF;

//-----------------------------------------------------------------------------
ChannelExtensionSPtr CountingFrameManager::createChannelExtension(ChannelExtension::Type type)
{
  Q_ASSERT(false);//TODO
}

//-----------------------------------------------------------------------------
SegmentationExtensionSPtr CountingFrameManager::createSegmentationExtension(SegmentationExtension::Type type)
{
  Q_ASSERT(false);//TODO
}

//------------------------------------------------------------------------
void CountingFrameManager::createAdaptiveCF(ChannelAdapterPtr channel,
                                            Nm inclusion[3],
                                            Nm exclusion[3])
{
  int id = 0;

  auto extension = retrieveOrCreateCFExtension(channel);

  auto cf = AdaptiveCountingFrame::New(id, extension, channel->bounds(), inclusion, exclusion);

  registerCountingFrame(cf, extension);
}

//------------------------------------------------------------------------
void CountingFrameManager::createRectangularCF(ChannelAdapterPtr channel,
                                               Nm inclusion[3],
                                               Nm exclusion[3])
{
  int id = 0;

  auto extension = retrieveOrCreateCFExtension(channel);

  auto cf = RectangularCountingFrame::New(id, extension, channel->bounds(), inclusion, exclusion);

  registerCountingFrame(cf, extension);
}

//-----------------------------------------------------------------------------
void CountingFrameManager::deleteCountingFrame ( CountingFrame* cf )
{
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
    extension =  new CountingFrameExtension();
    channel->addExtension(CountingFrameExtensionSPtr{extension});
  }
  Q_ASSERT(extension);

  return extension;
}


//-----------------------------------------------------------------------------
void CountingFrameManager::registerCountingFrame(CountingFrame* cf, CountingFrameExtension* extension)
{
  extension->addCountingFrame(cf);

  m_countingFrames << cf;

  emit countingFrameCreated(cf);
}
