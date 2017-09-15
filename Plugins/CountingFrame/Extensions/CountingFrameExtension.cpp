/*
    
    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>

    This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
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

// Plugin
#include "CountingFrameExtension.h"
#include "CountingFrames/CountingFrame.h"
#include <CountingFrameManager.h>

// ESPINA
#include <Core/Analysis/Segmentation.h>
#include <Core/Analysis/Query.h>
#include <Core/Analysis/Channel.h>
#include <Extensions/ExtensionUtils.h>
#include <Tasks/CountingFrameCreator.h>

// Qt
#include <QDebug>
#include <QApplication>

using namespace ESPINA;
using namespace ESPINA::Extensions;
using namespace ESPINA::Core;
using namespace ESPINA::CF;

StackExtension::Type CountingFrameExtension::TYPE = "CountingFrame";

//-----------------------------------------------------------------------------
CountingFrameExtension::CountingFrameExtension(CountingFrameManager *manager,
                                               SchedulerSPtr         scheduler,
                                               CoreFactory          *factory,
                                               const State          &state)
: StackExtension{InfoCache()}
, m_manager     {manager}
, m_scheduler   {scheduler}
, m_factory     {factory}
, m_prevState   {state}
{
}

//-----------------------------------------------------------------------------
CountingFrameExtension::~CountingFrameExtension()
{
  QWriteLocker lock(&m_CFmutex);

  for (auto cf : m_countingFrames)
  {
    m_manager->unregisterCountingFrame(cf);
  }
}

//-----------------------------------------------------------------------------
State CountingFrameExtension::state() const
{
  State state;

  QReadLocker lock(&m_CFmutex);

  QString br =  "";
  for(auto cf: m_countingFrames)
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
void CountingFrameExtension::deleteCountingFrame(CountingFrame* countingFrame)
{
  QWriteLocker lock(&m_CFmutex);

  Q_ASSERT(m_countingFrames.contains(countingFrame));
  for (auto segmentation : QueryContents::segmentationsOnChannelSample(m_extendedItem))
  {
    if(segmentation->extensions()->hasExtension(StereologicalInclusion::TYPE))
    {
      auto extension = retrieveExtension<StereologicalInclusion>(segmentation->extensions());
      extension->removeCountingFrame(countingFrame);
    }
  }

  m_countingFrames.removeOne(countingFrame);

  m_manager->unregisterCountingFrame(countingFrame);

  if (m_countingFrames.isEmpty())
  {
    safeDeleteExtension<StereologicalInclusion>(m_extendedItem);
  }

  delete countingFrame;
}

//-----------------------------------------------------------------------------
void CountingFrameExtension::onExtendedItemSet(Channel *channel)
{
  // only redo if no counting frames to allow changing the extended item.
  if(m_countingFrames.empty())
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
        }
        else
        {
          CFType type = static_cast<CFType>(params[TYPE_POS].toInt());

          Nm inclusion[3];
          Nm exclusion[3];

          for (int i = 0; i < 3; ++i)
          {
            inclusion[i] = params[INCLUSION_START_POS + i].toDouble();
            exclusion[i] = params[EXCLUSION_START_POS + i].toDouble();
          }

          createCountingFrame(type, inclusion, exclusion, params[CONSTRAINT_POS], params[ID_POS]);
        }
      }
    }
  }
}

//-----------------------------------------------------------------------------
void CountingFrameExtension::createCountingFrame(CFType type,
                                                 Nm inclusion[3],
                                                 Nm exclusion[3],
                                                 const QString &constraint,
                                                 const CountingFrame::Id &id)
{
  CountingFrameCreator::Data data;
  data.type       = type;
  data.inclusion  = NmVector3{inclusion[0], inclusion[1], inclusion[2]};
  data.exclusion  = NmVector3{exclusion[0], exclusion[1], exclusion[2]};
  data.extension  = this;
  data.id         = (id.isEmpty() ? m_manager->defaultCountingFrameId(constraint) : id);
  data.constraint = constraint;

  auto task = std::make_shared<CountingFrameCreator>(data, m_scheduler, m_factory);
  task->setDescription(tr("Creating CF %1: %2").arg(id.isEmpty() ? "" : id).arg(m_extendedItem->name()));

  connect(task.get(), SIGNAL(finished()),
          this,       SLOT(onCountingFrameCreated()));

  Task::submit(task);
}

//-----------------------------------------------------------------------------
void CountingFrameExtension::onCountingFrameCreated()
{
  auto task = qobject_cast<CountingFrameCreator *>(sender());

  if(task && !task->isAborted())
  {
    QWriteLocker lock(&m_CFmutex);

    auto cf   = task->getCountingFrame();
    auto data = task->getCountingFrameData();

    if(data.id.isEmpty())
    {
      data.id = m_manager->defaultCountingFrameId(data.constraint);
    }

    cf->setId(data.id);
    cf->setCategoryConstraint(data.constraint);

    m_countingFrames << cf;

    m_manager->registerCountingFrame(cf);
  }
}
