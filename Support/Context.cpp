/*

 Copyright 2015  Jorge Peña Pastor <jpena@cesvima.upm.es>

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

#include "Context.h"
#include <Core/MultiTasking/Scheduler.h>
#include <Core/Utils/EspinaException.h>
#include <GUI/ColorEngines/MultiColorEngine.h>
#include <Support/Utils/FactoryUtils.h>
#include <Support/Widgets/Panel.h>
#include <Support/Settings/Settings.h>
#include <Support/Representations/RepresentationFactory.h>

// Qt
#include <QMainWindow>

const int PERIOD_uSEC = 16000; // 16ms

using namespace ESPINA;
using namespace ESPINA::Core::Utils;
using namespace ESPINA::GUI::ColorEngines;
using namespace ESPINA::GUI::View;
using namespace ESPINA::Support;

//------------------------------------------------------------------------
Context::Context(QMainWindow *mainWindow, bool *minimizedStatus)
: m_viewState      {}
, m_model          {std::make_shared<ModelAdapter>()}
, m_activeROI      {std::make_shared<ROIAccumulator>()}
, m_scheduler      {std::make_shared<Scheduler>(PERIOD_uSEC)}
, m_factory        {std::make_shared<ModelFactory>(espinaCoreFactory(m_scheduler), m_scheduler)}
, m_colorEngine    {std::make_shared<MultiColorEngine>()}
, m_minimizedStatus{minimizedStatus}
, m_mainWindow     {mainWindow}
{
   QObject::connect(m_model.get(), SIGNAL(channelsRemoved(ViewItemAdapterSList)),
                   &m_viewState,   SLOT(updateSelection(ViewItemAdapterSList)));

   QObject::connect(m_model.get(), SIGNAL(segmentationsRemoved(ViewItemAdapterSList)),
                   &m_viewState,   SLOT(updateSelection(ViewItemAdapterSList)));

   ApplicationSettings settings;
   try
   {
     m_factory->setTemporalDirectory(QDir{settings.temporalPath()});
   }
   catch(...)
   {
     // nothing, automatically changed to QDir::temp().
   }
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
ROIAccumulatorSPtr Context::roiProvider() const
{
  return m_activeROI;
}

//------------------------------------------------------------------------
const RepresentationFactorySList Context::availableRepresentations() const
{
  return m_representations.keys();
}

//------------------------------------------------------------------------
const RepresentationList Context::representations() const
{
  return m_representations.values();
}

//------------------------------------------------------------------------
const Representation Context::representation(const RepresentationFactorySPtr factory) const
{
  if(!m_representations.contains(factory))
  {
    auto message = QObject::tr("Asked for an unknown representation.");
    auto details = QObject::tr("Context::representation(factory) -> ") + message;

    throw EspinaException(message, details);
  }

  return m_representations[factory];
}

//------------------------------------------------------------------------
const Representation &Context::addRepresentation(const RepresentationFactorySPtr factory)
{
  if(!m_representations.keys().contains(factory))
  {
    auto representation = factory->createRepresentation(*this, ViewType::VIEW_2D|ViewType::VIEW_3D);

    m_representations.insert(factory, representation);
  }

  return m_representations[factory];
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
ColorEngineSPtr Context::colorEngine() const
{
  return m_colorEngine;
}

//------------------------------------------------------------------------
void Context::addColorEngine(ColorEngineSPtr engine)
{
  m_colorEngine->add(engine);
}

//------------------------------------------------------------------------
void Context::addPanel(Panel *panel)
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
Context &WithContext::getContext() const
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
