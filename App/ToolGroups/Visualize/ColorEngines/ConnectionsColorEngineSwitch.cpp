/*

 Copyright (C) 2018 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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

// ESPINA
#include <App/ToolGroups/Visualize/ColorEngines/ConnectionsColorEngineSwitch.h>
#include <Core/Utils/ListUtils.hxx>
#include <GUI/ColorEngines/ConnectionsColorEngine.h>

// Qt
#include <QIcon>
#include <QThread>
#include <QDebug>

using namespace ESPINA;
using namespace ESPINA::Core::Utils;
using namespace ESPINA::GUI::ColorEngines;

//--------------------------------------------------------------------
ConnectionsColorEngineSwitch::ConnectionsColorEngineSwitch(Support::Context& context)
: ColorEngineSwitch{std::make_shared<ConnectionsColorEngine>(), QIcon{":/espina/connectionGradient.svg"}, context}
, m_needUpdate{true}
{
  connect(this, SIGNAL(toggled(bool)),
          this, SLOT(onToolToggled(bool)));
}

//--------------------------------------------------------------------
ConnectionsColorEngineSwitch::~ConnectionsColorEngineSwitch()
{
  if(isChecked()) onToolToggled(false);

  abortTask();
}

//--------------------------------------------------------------------
void ConnectionsColorEngineSwitch::restoreSettings(std::shared_ptr<QSettings> settings)
{
  restoreCheckedState(settings);
}

//--------------------------------------------------------------------
void ConnectionsColorEngineSwitch::saveSettings(std::shared_ptr<QSettings> settings)
{
  saveCheckedState(settings);
}

//--------------------------------------------------------------------
void ConnectionsColorEngineSwitch::onToolToggled(bool checked)
{
  if (checked)
  {
    updateRange();

    connect(getModel().get(), SIGNAL(segmentationsAdded(ViewItemAdapterSList)),
            this,             SLOT(onRangeModified()));

    connect(getModel().get(), SIGNAL(segmentationsRemoved(ViewItemAdapterSList)),
            this,             SLOT(onRangeModified()));
  }
  else
  {
    disconnect(getModel().get(), SIGNAL(segmentationsAdded(ViewItemAdapterSList)),
               this,             SLOT(onRangeModified()));

    disconnect(getModel().get(), SIGNAL(segmentationsRemoved(ViewItemAdapterSList)),
               this,             SLOT(onRangeModified()));
  }
}

//--------------------------------------------------------------------
void ConnectionsColorEngineSwitch::onTaskFinished()
{
  auto task = qobject_cast<UpdateConnectionsRangeTask *>(sender());

  if(m_task.get() == task)
  {
    m_task = nullptr;
  }
}

//--------------------------------------------------------------------
void ConnectionsColorEngineSwitch::onRangeModified()
{
  m_needUpdate = true;

  if(isChecked()) updateRange();
}

//--------------------------------------------------------------------
void ConnectionsColorEngineSwitch::abortTask()
{
  if(m_task != nullptr)
  {
    disconnect(m_task.get(), SIGNAL(finished()), this, SLOT(onTaskFinished()));

    m_task->abort();

    if(!m_task->thread()->wait(100))
    {
      m_task->thread()->terminate();
    }

    m_task = nullptr;
  }
}

//--------------------------------------------------------------------
void ConnectionsColorEngineSwitch::updateRange()
{
  if(m_needUpdate)
  {
    m_needUpdate = false;

    if (m_task && m_task->isRunning())
    {
      m_task->abort();
    }

    auto engine = std::dynamic_pointer_cast<ConnectionsColorEngine>(colorEngine());

    m_task = std::make_shared<UpdateConnectionsRangeTask>(getModel()->segmentations(), engine.get(), getScheduler());
    showTaskProgress(m_task);

    connect(m_task.get(), SIGNAL(finished()), this, SLOT(onTaskFinished()));

    Task::submit(m_task);
  }
}

//--------------------------------------------------------------------
UpdateConnectionsRangeTask::UpdateConnectionsRangeTask(SegmentationAdapterSList segmentations,
                                                       GUI::ColorEngines::ConnectionsColorEngine *engine,
                                                       SchedulerSPtr scheduler)
: Task           {scheduler}
, m_segmentations{segmentations}
, m_engine       {engine}
{
}

//--------------------------------------------------------------------
void UpdateConnectionsRangeTask::run()
{
  unsigned int min   = std::numeric_limits<unsigned int>::max();
  unsigned int max   = std::numeric_limits<unsigned int>::min();
  unsigned int i     = 0;
  unsigned int total = m_segmentations.size();

  for(auto segmentation : m_segmentations)
  {
    if (!canExecute()) return;

    auto model = segmentation->model();
    auto connections = model->connections(segmentation);
    unsigned int number = connections.size();

    min = std::min(min, number);
    max = std::max(max, number);

    auto progress = static_cast<double>(i++)/total*100;
    reportProgress(progress);
  }

  if(canExecute() && !isAborted())
  {
    m_engine->setRange(min, max);
  }
}
