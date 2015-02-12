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
#include "Widgets/ToolGroup.h"
#include <Core/Utils/Measure.h>
#include <GUI/View/RenderView.h>
#include <App/ToolGroups/View/ViewToolGroup.h>

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
: m_timer            {new Timer{1}}
, m_viewState        {new ViewState{m_timer}}
, m_sourcesState     {new PipelineSourcesState{m_timer}}
, m_selection        {new Selection()}
, m_roiProvider      {nullptr}
, m_contextualToolBar{nullptr}
, m_toolGroup        {nullptr}
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

  m_sourcesState->addSource(&m_segmentationSources);
  m_sourcesState->addSource(&m_channelSources);
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

  view->setState(m_viewState);
  view->setEventHandler(m_eventHandler);
  //view->setColorEngine(m_colorEngine);

  view->setChannelSources(&m_channelSources);

  for (auto manager : m_repManagers)
  {
    if (manager->supportedViews().testFlag(view->type()))
    {
      view->addRepresentationManager(manager->clone());
    }
  }

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

//----------------------------------------------------------------------------
void ViewManager::addRepresentationPools(const QString& group, RepresentationPoolSList pools)
{
  if (ViewToolGroup::CHANNELS_GROUP == group)
  {
    for (auto pool : pools)
    {
      pool->setPipelineSources(&m_channelSources);
      m_channelPools << pool;
    }
  }
}

//----------------------------------------------------------------------------
void ViewManager::addRepresentationManagers(RepresentationManagerSList repManagers)
{
  for (auto repManager : repManagers)
  {
    for (auto renderView : m_renderViews)
    {
      if (repManager->supportedViews().testFlag(renderView->type()))
      {
        renderView->addRepresentationManager(repManager->clone());
      }
    }
    m_repManagers << repManager;
  }
}

//---------------------------------------------------------------------------
/********************* ViewItem Management API *****************************/
//---------------------------------------------------------------------------

//----------------------------------------------------------------------------
void ViewManager::add(ChannelAdapterPtr channel)
{
  m_channelSources.addSource(channel);
}

//----------------------------------------------------------------------------
void ViewManager::add(SegmentationAdapterPtr segmentation)
{
  m_segmentationSources.addSource(segmentation);
}

//----------------------------------------------------------------------------
void ViewManager::remove(ChannelAdapterPtr channel)
{
  m_channelSources.removeSource(channel);
}

//----------------------------------------------------------------------------
void ViewManager::remove(SegmentationAdapterPtr segmentation)
{
  m_segmentationSources.removeSource(segmentation);
}

//----------------------------------------------------------------------------
bool ViewManager::updateRepresentation(ChannelAdapterPtr channel, bool render) // TODO: Remove render flag
{
  m_channelSources.onSourceUpdated(channel);

  return true; // TODO Void


  // TODO: Seguramente como parte del nuevo algoritmo de render de la vista
//   if (!m_sceneCameraInitialized && state.visible)
//   {
//     m_sceneCameraInitialized = true;
//     resetCamera();
//   }
//
//   m_renderer->ResetCameraClippingRange();
//
//   if (render && isVisible())
//   {
//     m_view->GetRenderWindow()->Render();
//     m_view->update();
//   }
}

//----------------------------------------------------------------------------
bool ViewManager::updateRepresentation(SegmentationAdapterPtr seg, bool render)
{
  Q_ASSERT(false);
  return false;
}

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
void ViewManager::displayTools(ToolGroupPtr group)
{
  if (m_toolGroup)
  {
    m_toolGroup->showTools(false);
  }

  hideTools(m_toolGroup);

  m_toolGroup = group;

  if (m_contextualToolBar)
  {
    for(auto tool : group->tools())
    {
      for(auto action : tool->actions())
      {
        m_contextualToolBar->addAction(action);
      }
    }
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
    view->updateView();
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
//     for(auto item : seg->relatedItems(ESPINA::RELATION_OUT))
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
//       foreach(ModelItemSPtr item, segmentation->relatedItems(ESPINA::RELATION_OUT))
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

  for(auto view: m_renderViews)
  {
   // TODO view->setColorEngine(engine);
  }

  updateSegmentationRepresentations();
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
