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
#include <GUI/ColorEngines/MultiColorEngine.h>
#include <QUndoStack>


const int PERIOD_uSEC = 16000; // 16ms

using namespace ESPINA;
using namespace ESPINA::GUI::View;
using namespace ESPINA::Support;

//------------------------------------------------------------------------
Context::Context()
: ActiveChannel(nullptr)
, m_invalidathor(m_timer)
, m_viewState(m_timer, m_invalidathor)
, m_model(new ModelAdapter())
, m_activeROI(new ROIAccumulator())
, m_scheduler(new Scheduler(PERIOD_uSEC))
, m_selection(new Selection())
, m_colorEngine(std::make_shared<MultiColorEngine>())
, m_undoStack(new QUndoStack())
, m_factory(new ModelFactory(espinaCoreFactory(m_scheduler), m_scheduler))
{
  QObject::connect(m_model.get(), SIGNAL(modelChanged()),
                  &m_timer,       SLOT(increment()));
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
GUI::View::ViewState &Context::viewState()
{
  return m_viewState;
}

//------------------------------------------------------------------------
SelectionSPtr Context::selection() const
{
  return m_selection;
}

//------------------------------------------------------------------------
ColorEngineSPtr Context::colorEngine() const
{
  return m_colorEngine;
}

//------------------------------------------------------------------------
Timer &Context::timer()
{
  return m_viewState.timer();
}

//------------------------------------------------------------------------
RepresentationInvalidator &Context::representationInvalidator()
{
  return m_viewState.representationInvalidator();
}
