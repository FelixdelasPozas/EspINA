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
#include "Widgets/DockWidget.h"
#include <Core/MultiTasking/Scheduler.h>
#include <GUI/ColorEngines/MultiColorEngine.h>
#include <QUndoStack>
#include <QMainWindow>


const int PERIOD_uSEC = 16000; // 16ms

using namespace ESPINA;
using namespace ESPINA::GUI::ColorEngines;
using namespace ESPINA::GUI::View;
using namespace ESPINA::Support;

//------------------------------------------------------------------------
Context::Context(QMainWindow *mainWindow, bool *minimizedStatus)
: m_invalidator(m_timer)
, m_viewState(m_timer, m_invalidator)
, m_model(new ModelAdapter())
, m_activeROI(new ROIAccumulator())
, m_scheduler(new Scheduler(PERIOD_uSEC))
, m_factory(new ModelFactory(espinaCoreFactory(m_scheduler), m_scheduler, &m_invalidator))
, m_colorEngine(std::make_shared<MultiColorEngine>())
, m_minimizedStatus(minimizedStatus)
, m_mainWindow(mainWindow)
{
  QObject::connect(m_model.get(), SIGNAL(modelChanged()),
                  &m_timer,       SLOT(increment()));
}

//------------------------------------------------------------------------
Context::~Context()
{
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
ROIAccumulatorSPtr Context::roiProvider()
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
QUndoStack *Context::undoStack()
{
  return &m_undoStack;
}

//------------------------------------------------------------------------
GUI::View::ViewState &Context::viewState()
{
  return m_viewState;
}

//------------------------------------------------------------------------
MultiColorEngineSPtr Context::colorEngine() const
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

//------------------------------------------------------------------------
void Context::addPanel(DockWidget *panel)
{
  m_mainWindow->addDockWidget(Qt::DockWidgetArea::LeftDockWidgetArea, panel);
}

//------------------------------------------------------------------------
bool Context::isMinimized() const
{
  return *m_minimizedStatus;
}

//------------------------------------------------------------------------
SelectionSPtr Support::getSelection(Context &context)
{
  return context.viewState().selection();
}

//------------------------------------------------------------------------
ChannelAdapterPtr Support::getActiveChannel(Context &context)
{
  return getSelection(context)->activeChannel();
}

//------------------------------------------------------------------------
SegmentationAdapterList Support::getSelectedSegmentations(Context &context)
{
  return getSelection(context)->segmentations();
}

//------------------------------------------------------------------------
WithContext::WithContext(Context &context)
: m_context(context)
{

}

//------------------------------------------------------------------------
Context &WithContext::context()
{
  return m_context;
}

//------------------------------------------------------------------------
ChannelAdapterPtr WithContext::getActiveChannel() const
{
  return Support::getActiveChannel(m_context);
}

//------------------------------------------------------------------------
SelectionSPtr WithContext::getSelection() const
{
  return Support::getSelection(m_context);
}

//------------------------------------------------------------------------
SegmentationAdapterList WithContext::getSelectedSegmentations() const
{
  return getSelection()->segmentations();
}

//------------------------------------------------------------------------
ModelFactorySPtr WithContext::getFactory() const
{
  return m_context.factory();
}

//------------------------------------------------------------------------
SchedulerSPtr WithContext::getScheduler() const
{
  return m_context.scheduler();
}

//------------------------------------------------------------------------
ModelAdapterSPtr WithContext::getModel() const
{
  return m_context.model();
}

//------------------------------------------------------------------------
ESPINA::GUI::View::ViewState &WithContext::getViewState() const
{
  return m_context.viewState();
}

//------------------------------------------------------------------------
QUndoStack *WithContext::getUndoStack() const
{
  return m_context.undoStack();
}
