/*
 * Copyright 2015  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include "Context.h"
#include "Utils/FactoryUtils.h"
#include <Core/MultiTasking/Scheduler.h>
#include <QUndoStack>


const int PERIOD_uSEC = 16000; // 16ms

using namespace ESPINA;
using namespace ESPINA::GUI::View;
using namespace ESPINA::Support;

//------------------------------------------------------------------------
Context::Context()
: ActiveChannel(nullptr)
, m_timer(new Timer(1))
, m_scheduler(new Scheduler(PERIOD_uSEC))
, m_model(new ModelAdapter(m_timer))
, m_activeROI(new ROIAccumulator())
, m_viewState(new ViewState(std::make_shared<CoordinateSystem>(), m_timer))
, m_undoStack(new QUndoStack())
, m_factory(new ModelFactory(espinaCoreFactory(m_scheduler), m_scheduler))
{

}

//------------------------------------------------------------------------
Context::~Context()
{
  delete m_undoStack;
}

//------------------------------------------------------------------------
SchedulerSPtr Context::scheduler() const
{
  return m_scheduler;
}

//------------------------------------------------------------------------
ModelAdapterSPtr Context::model() const
{
  return m_model;
}

//------------------------------------------------------------------------
ROIAccumulatorSPtr Context::activeROI() const
{
  return m_activeROI;
}

//------------------------------------------------------------------------
RepresentationFactorySList &Context::availableRepresentations()
{
  return m_availableRepresentations;
}

//------------------------------------------------------------------------
ModelFactorySPtr Context::factory() const
{
  return m_factory;
}

//------------------------------------------------------------------------
QUndoStack *Context::undoStack() const
{
  return m_undoStack;
}

//------------------------------------------------------------------------
ViewStateSPtr Context::viewState() const
{
  return m_viewState;
}

//------------------------------------------------------------------------
ColorEngineSPtr Context::colorEngine() const
{
}
