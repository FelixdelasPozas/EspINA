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

// ESPINA
#include "ViewManager.h"
#include <Core/Utils/Measure.h>
#include <GUI/View/RenderView.h>

// VTK
#include <vtkMath.h>

// Qt
#include <QSettings>
#include <QString>
#include <QStack>
#include <QDebug>
#include <QToolBar>

using namespace ESPINA;

const QString FIT_TO_SLICES ("ViewManager::FitToSlices");

//----------------------------------------------------------------------------
ViewManager::ViewManager()
: m_selection        {new Selection()}
, m_roiProvider      {nullptr}
, m_eventHandler     {nullptr}
, m_viewResolution   {1., 1., 1.}
, m_resolutionUnits  {QString("nm")}
, m_activeChannel    {nullptr}
, m_activeCategory   {nullptr}
{
//   QSettings settings(CESVIMA, ESPINA_SETTINGS_SETTINGS);
  bool fitEnabled = true;

//   if (!settings.allKeys().contains(FIT_TO_SLICES))
//   {
//     settings.setValue(FIT_TO_SLICES, true);
//     fitEnabled = true;
//   }
//   else
//     fitEnabled = settings.value(FIT_TO_SLICES, true).toBool();

  m_fitToSlices = new QAction(tr("Fit To Slices"), this);
  m_fitToSlices->setCheckable(true);
  m_fitToSlices->setChecked(fitEnabled);

  connect(m_fitToSlices, SIGNAL(toggled(bool)),
          this,          SLOT(setFitToSlices(bool)));
}

//----------------------------------------------------------------------------
ViewManager::~ViewManager()
{
//   qDebug() << "********************************************************";
//   qDebug() << "              Destroying View Manager";
//   qDebug() << "********************************************************";
}

//----------------------------------------------------------------------------
void ViewManager::registerView(SelectableView* view)
{
  Q_ASSERT(!m_espinaViews.contains(view));
  view->setSharedSelection(m_selection);
  m_espinaViews << view;
}

//----------------------------------------------------------------------------
void ViewManager::registerView(RenderView* view)
{
  registerView(static_cast<SelectableView *>(view));

  Q_ASSERT(!m_renderViews.contains(view));
  m_renderViews << view;

  view->setEventHandler(m_eventHandler);

  for (auto widget : m_widgets)
  {
    view->addWidget(widget);
  }
}

//----------------------------------------------------------------------------
void ViewManager::unregisterView(SelectableView* view)
{
  Q_ASSERT(m_espinaViews.contains(view));
  m_espinaViews.removeAll(view);
}

//----------------------------------------------------------------------------
void ViewManager::unregisterView(RenderView* view)
{
  Q_ASSERT(m_renderViews.contains(view));

  m_renderViews.removeAll(view);
  unregisterView(static_cast<SelectableView *>(view));

  view->setEventHandler(EventHandlerSPtr());
  view->setState(std::make_shared<ViewState>());

  for(auto widget: m_widgets)
  {
    view->removeWidget(widget);
  }
}

//----------------------------------------------------------------------------
QList<View2D *> ViewManager::sliceViews()
{
  QList<View2D *> views;
  for(auto view: m_renderViews)
  {
    auto view2d = dynamic_cast<View2D *>(view);
    if(view2d != nullptr)
      views << view2d;
  }

  return views;
}

//---------------------------------------------------------------------------
/********************* ViewItem Management API *****************************/
//---------------------------------------------------------------------------
//----------------------------------------------------------------------------
void ViewManager::removeAllViewItems()
{
}

//----------------------------------------------------------------------------
void ViewManager::setSelectionEnabled(bool enable)
{
  for(auto view: m_renderViews)
  {
    view->setSelectionEnabled(enable);
  }
}

//----------------------------------------------------------------------------
void ViewManager::setSelection(ViewItemAdapterList selection)
{
  m_selection->set(selection);

  emit selectionChanged(m_selection);
}

//----------------------------------------------------------------------------
void ViewManager::setEventHandler(EventHandlerSPtr eventHandler)
{
  if(m_eventHandler == eventHandler)
    return;

  if (m_eventHandler)
  {
    m_eventHandler->setInUse(false);
  }

  m_eventHandler = eventHandler;

  if (m_eventHandler)
  {
    m_eventHandler->setInUse(true);
  }

  for(auto view : m_renderViews)
  {
    view->setEventHandler(m_eventHandler);
  }

  emit eventHandlerChanged();
}

//----------------------------------------------------------------------------
void ViewManager::unsetActiveEventHandler()
{
  unsetEventHandler(m_eventHandler);
}

//----------------------------------------------------------------------------
void ViewManager::unsetEventHandler(EventHandlerSPtr eventHandler)
{
  if (eventHandler && m_eventHandler == eventHandler)
  {
    m_eventHandler->setInUse(false);
    m_eventHandler.reset();

    for(auto view : m_renderViews)
    {
      view->setEventHandler(m_eventHandler);
    }
  }
}

//----------------------------------------------------------------------------
void ViewManager::updateViews()
{
  for(auto view: m_renderViews)
  {
    view->updateView();
  }
}

//----------------------------------------------------------------------------
void ViewManager::setFitToSlices(bool enabled)
{
//   QSettings settings(CESVIMA, ESPINA_SETTINGS_SETTINGS);
//   settings.setValue(FIT_TO_SLICES, enabled);
//   settings.sync();
  for(auto view: m_renderViews)
  {
    auto view2d = dynamic_cast<View2D*>(view);
    if (view2d != nullptr)
      view2d->setFitToSlices(enabled);
  }
}

//----------------------------------------------------------------------------
void ViewManager::setActiveChannel(ChannelAdapterPtr channel)
{
  m_activeChannel = channel;

  emit activeChannelChanged(channel);
}
//----------------------------------------------------------------------------
void ViewManager::addWidget(EspinaWidgetSPtr widget)
{
  if(m_widgets.contains(widget))
    return;

  for(auto view: m_renderViews)
    view->addWidget(widget);

  m_widgets << widget;
}

//----------------------------------------------------------------------------
void ViewManager::removeWidget(EspinaWidgetSPtr widget)
{
  if(!m_widgets.contains(widget))
    return;

  for(auto view: m_renderViews)
    view->removeWidget(widget);

  m_widgets.removeOne(widget);
}

//----------------------------------------------------------------------------
void ViewManager::resetViewCameras()
{
  for(auto view: m_renderViews)
  {
    view->resetCamera();
  }
}

//----------------------------------------------------------------------------
void ViewManager::setCrosshairVisibility(bool value)
{
  for(auto view: sliceViews())
  {
    view->setCrosshairVisibility(value);
  }
}

//----------------------------------------------------------------------------
void ViewManager::addSliceSelectors(SliceSelectorSPtr widget,
                                    View2D::SliceSelectionType selectors)
{
  for(auto view : sliceViews())
  {
    view->addSliceSelectors(widget, selectors);
  }
}

//----------------------------------------------------------------------------
void ViewManager::removeSliceSelectors(SliceSelectorSPtr widget)
{
  for(auto view : sliceViews())
  {
    view->removeSliceSelectors(widget);
  }
}


//----------------------------------------------------------------------------
void ViewManager::setColorEngine(ColorEngineSPtr engine)
{
  m_colorEngine = engine;
}

//----------------------------------------------------------------------------
void ViewManager::focusViewsOn(const NmVector3& point)
{
  m_viewState->focusViewOn(point);
}

//----------------------------------------------------------------------------
NmVector3 ViewManager::viewResolution()
{
  if (m_renderViews.empty())
    return NmVector3{1., 1., 1.};

  // ASSERT: the first view is not from type VOLUME and all the views have to
  // have the same view resolution, so we can get the correct resolution from
  // any of them
  m_viewResolution = m_renderViews.first()->sceneResolution();

  double smaller = std::min(m_viewResolution[0], std::min(m_viewResolution[1], m_viewResolution[2]));
  m_resolutionUnits = Measure(smaller).getUnits();

  return m_viewResolution;
}

//----------------------------------------------------------------------------
MeasureSPtr ViewManager::measure(Nm distance)
{
  MeasurePtr measure = new Measure(distance);
  viewResolution();
  measure->toUnits(m_resolutionUnits);

  return MeasureSPtr(measure);
}
