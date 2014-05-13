/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2011  Jorge Peña Pastor <jpena@cesvima.upm.es>

    This program is free software: you can redistribute it and/or modify
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

#include "ViewManager.h"
#include "ToolGroup.h"

// EspINA
#include <Core/Utils/Measure.h>
#include <GUI/View/RenderView.h>
#include <GUI/View/View2D.h>

#include <vtkMath.h>

// Qt
#include <QSettings>
#include <QString>
#include <QStack>
#include <QDebug>
#include <QToolBar>

using namespace EspINA;

const QString FIT_TO_SLICES ("ViewManager::FitToSlices");

//----------------------------------------------------------------------------
ViewManager::ViewManager()
: m_selection        {new Selection()}
, m_roi              {nullptr}
, m_contextualToolBar{nullptr}
, m_toolGroup        {nullptr}
, m_eventHandler     {nullptr}
, m_viewResolution   {1., 1., 1.}
, m_resolutionUnits  {QString("nm")}
, m_activeChannel    {nullptr}
, m_activeCategory   {nullptr}
{
//   QSettings settings(CESVIMA, ESPINA);
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
  Q_ASSERT(!m_renderViews.contains(view));
  m_renderViews << view;
  registerView(static_cast<SelectableView *>(view));

  view->setEventHandler(m_eventHandler);
  view->setColorEngine(m_colorEngine);
}

//----------------------------------------------------------------------------
void ViewManager::registerView(View2D* view)
{
  Q_ASSERT(!m_sliceViews.contains(view));
  m_sliceViews << view;
  registerView(static_cast<RenderView *>(view));
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
  Q_ASSERT(m_espinaViews.contains(view));
  m_espinaViews.removeAll(view);
}

//----------------------------------------------------------------------------
void ViewManager::unregisterView(View2D* view)
{
  Q_ASSERT(m_renderViews.contains(view));
  m_renderViews.removeAll(view);
  Q_ASSERT(m_espinaViews.contains(view));
  m_espinaViews.removeAll(view);
  Q_ASSERT(m_sliceViews.contains(view));
  m_sliceViews.removeAll(view);
}

//----------------------------------------------------------------------------
void ViewManager::setSelectionEnabled(bool enable)
{
  for(auto view: m_renderViews)
    view->setSelectionEnabled(enable);
}

//----------------------------------------------------------------------------
void ViewManager::setSelection(ViewItemAdapterList selection)
{
  m_selection->set(selection);

  emit selectionChanged(m_selection);
  //TODO 2012-10-07 computeSelectionCenter();
}

//----------------------------------------------------------------------------
void ViewManager::displayTools(ToolGroupPtr group)
{
  if (m_toolGroup)
  {
    m_toolGroup->showTools(false);
  }

  hideTools(m_toolGroup);

  m_toolGroup = group;

  //QAction *separator; // Cheap way of removing last separator without an 'if' in the external loop
  if (m_contextualToolBar)
  {
    for(auto tool : group->tools())
    {
      for(auto action : tool->actions())
      {
        m_contextualToolBar->addAction(action);
      }
      //separator = m_contextualToolBar->addSeparator();
    }
    //m_contextualToolBar->removeAction(separator);
  }
}

//----------------------------------------------------------------------------
void ViewManager::hideTools(ToolGroupPtr group)
{
  if (m_toolGroup == group)
  {
    m_toolGroup = nullptr;

    if (m_contextualToolBar)
    {
      m_contextualToolBar->clear();
    }
  }
}

//----------------------------------------------------------------------------
void ViewManager::setEventHandler(EventHandlerSPtr eventHandler)
{
  if (m_eventHandler)
  {
    m_eventHandler->setInUse(false);
  }

  m_eventHandler = eventHandler;

  if (m_eventHandler)
    m_eventHandler->setInUse(true);

  for(auto view : m_renderViews)
  {
    view->setEventHandler(m_eventHandler);
  }

  emit eventHandlerChanged();
}

//----------------------------------------------------------------------------
void ViewManager::unsetEventHandler(EventHandlerSPtr eventHandler)
{
  if (m_eventHandler == eventHandler)
  {
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
    view->updateView();
}

//----------------------------------------------------------------------------
void ViewManager::setFitToSlices(bool enabled)
{
//   QSettings settings(CESVIMA, ESPINA);
//   settings.setValue(FIT_TO_SLICES, enabled);
//   settings.sync();
  for(auto view: m_sliceViews)
    view->setFitToSlices(enabled);
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
  for(auto view: m_renderViews)
    view->addWidget(widget);
}

//----------------------------------------------------------------------------
void ViewManager::removeWidget(EspinaWidgetSPtr widget)
{
  for(auto view: m_renderViews)
    view->removeWidget(widget);
}

//----------------------------------------------------------------------------
void ViewManager::resetViewCameras()
{
  for(auto view: m_renderViews)
    view->resetCamera();
}

//----------------------------------------------------------------------------
void ViewManager::updateSegmentationRepresentations(SegmentationAdapterPtr segmentation)
{
  QStack<SegmentationAdapterPtr> stack;
  SegmentationAdapterList segmentations;

  stack.push_front(segmentation);
  segmentations << segmentation;

//   while (!stack.isEmpty())
//   {
//     SegmentationPtr seg = stack.pop();
//     for(auto item : seg->relatedItems(EspINA::RELATION_OUT))
//     {
//       if (item->type() == SEGMENTATION)
//       {
//         SegmentationPtr relatedSeg = segmentationPtr(item.get());
//         if (relatedSeg->isInputSegmentationDependent() && !segmentations.contains(relatedSeg))
//         {
//           stack.push_front(relatedSeg);
//           segmentations << relatedSeg;
//         }
//       }
//     }
//   }

  for(auto view: m_espinaViews)
  {
    view->updateRepresentations(segmentations);
  }
}

//----------------------------------------------------------------------------
void ViewManager::updateSegmentationRepresentations(SegmentationAdapterList list)
{
  QStack<SegmentationAdapterPtr> itemsStack;
  SegmentationAdapterList segList;

  for(auto seg: list)
  {
    if (segList.contains(seg))
      continue;

    itemsStack.push_front(seg);
    segList << seg;

//     while (!itemsStack.isEmpty())
//     {
//       SegmentationPtr segmentation = itemsStack.pop();
//       foreach(ModelItemSPtr item, segmentation->relatedItems(EspINA::RELATION_OUT))
//         if (item->type() == SEGMENTATION)
//         {
//           SegmentationPtr relatedSeg = segmentationPtr(item.get());
//           if (relatedSeg->isInputSegmentationDependent() && !segList.contains(relatedSeg))
//           {
//             itemsStack.push_front(relatedSeg);
//             segList << relatedSeg;
//           }
//         }
//     }
  }

  for(auto view: m_espinaViews)
  {
    view->updateRepresentations(segList);
  }
}

//----------------------------------------------------------------------------
void ViewManager::updateChannelRepresentations(ChannelAdapterList list)
{
  for(auto view: m_espinaViews)
    view->updateRepresentations(list);
}

//----------------------------------------------------------------------------
void ViewManager::setSegmentationVisibility(bool visible)
{
  for(auto view: m_renderViews)
  {
    view->setSegmentationsVisibility(visible);
  }
}

//----------------------------------------------------------------------------
void ViewManager::setCrosshairVisibility(bool value)
{
  for(auto view: m_sliceViews)
  {
    view->setCrosshairVisibility(value);
  }
}

// //----------------------------------------------------------------------------
// void ViewManager::addSliceSelectors(SliceSelectorWidget* widget,
//                                     ViewManager::SliceSelectors selectors)
// {
//   foreach(View2D *view, m_sliceViews)
//     view->addSliceSelectors(widget, selectors);
// }
// 
// //----------------------------------------------------------------------------
// void ViewManager::removeSliceSelectors(SliceSelectorWidget* widget)
// {
//   foreach(View2D *view, m_sliceViews)
//     view->removeSliceSelectors(widget);
// }
// 

//----------------------------------------------------------------------------
void ViewManager::setColorEngine(ColorEngineSPtr engine)
{
  m_colorEngine = engine;

  for(auto view: m_renderViews)
    view->setColorEngine(engine);

  updateSegmentationRepresentations();
}

//----------------------------------------------------------------------------
void ViewManager::focusViewsOn(const NmVector3& point)
{
  for(auto view: m_renderViews)
    view->centerViewOn(point, true);
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
