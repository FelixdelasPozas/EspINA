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
#include "CountingFrames/AdaptiveCountingFrame.h"
#include "CountingFrames/OrtogonalCountingFrame.h"
#include <Core/Analysis/Channel.h>
#include <Core/Analysis/Category.h>

using namespace EspINA;
using namespace EspINA::CF;

//------------------------------------------------------------------------
CountingFrameExtensionSPtr CountingFrameManager::createExtension(const State& state) const
{
  return CountingFrameExtensionSPtr{new CountingFrameExtension(this, state)};
}

// //------------------------------------------------------------------------
// void CountingFrameManager::createAdaptiveCF(ChannelAdapterPtr channel,
//                                             Nm inclusion[3],
//                                             Nm exclusion[3],
//                                             const QString &constraint)
// {
//   auto extension = retrieveOrCreateCFExtension(channel);
//
//   auto cf = AdaptiveCountingFrame::New(extension, channel->bounds(), inclusion, exclusion);
//
//   cf->setCategoryConstraint(constraint);
//
//   registerCountingFrame(cf, extension);
// }
//
// //------------------------------------------------------------------------
// void CountingFrameManager::createRectangularCF(ChannelAdapterPtr channel,
//                                                Nm inclusion[3],
//                                                Nm exclusion[3],
//                                             const QString &constraint)
// {
//   auto extension = retrieveOrCreateCFExtension(channel);
//
//   auto cf = OrtogonalCountingFrame::New(extension, channel->bounds(), inclusion, exclusion);
//
//   cf->setCategoryConstraint(constraint);
//
//   registerCountingFrame(cf, extension);
// }

// //-----------------------------------------------------------------------------
// void CountingFrameManager::deleteCountingFrame(CountingFrame* cf)
// {
//   Q_ASSERT(m_countingFrames.contains(cf));
//
//
//
// //   while (!cfExtension && i < m_countingFramesExtensions.size())
// //   {
// //     if (m_countingFramesExtensions[i]->countingFrames().contains(cf))
// //     {
// //       cfExtension = m_countingFramesExtensions[i];
// //       cfExtension->deleteCountingFrame(cf);
// //     }
// //     else
// //       ++i;
// //   }
// //   m_countingFramesExtensions.removeAt(i);
// //   m_countingFrames.removeOne(cf);
//
//   m_countingFrames.remove(cf);
//   Q_ASSERT(!m_countingFrames.contains(cf));
//
//   cf->Delete();
// }

//-----------------------------------------------------------------------------
void CountingFrameManager::registerCountingFrame(CountingFrame* cf)
{
  m_countingFrames[cf] = cf->extension()->extendedItem();

  emit countingFrameCreated(cf);
}

//-----------------------------------------------------------------------------
void CountingFrameManager::unregisterCountingFrame(CountingFrame* cf)
{
  m_countingFrames.remove(cf);
}


//-----------------------------------------------------------------------------
CountingFrame::Id CountingFrameManager::defaultCountingFrameId(const QString &constraint) const
{
  CountingFrame::Id id = constraint;

  if (id.isEmpty())
  {
    id = "Global";
  }

  return suggestedId(id);
}

//-----------------------------------------------------------------------------
CountingFrame::Id CountingFrameManager::suggestedId(const CountingFrame::Id id) const
{
  QRegExp cardinalityRegExp("\\([0-9]+\\)");
  QString cardinalityStrippedId = id.replace(cardinalityRegExp, "").trimmed();

  QString suggestedId = cardinalityStrippedId;

  int count = 0;
  for (auto cf : m_countingFrames.keys())
  {
    if (cf->id().startsWith(cardinalityStrippedId))
    {
      int cardinalityIndex = cf->id().lastIndexOf(cardinalityRegExp);

      if (cardinalityIndex == -1)
      {
        ++count;
      }
      else
      {
        auto cardinality = cf->id().mid(cardinalityIndex + 1);
        cardinality = cardinality.left(cardinality.length()-1);

        count = std::max(count, cardinality.toInt() + 1);
      }
    }
  }

  if (count > 0)
  {
    suggestedId.append(QString(" (%1)").arg(count));
  }

  return suggestedId;
}
