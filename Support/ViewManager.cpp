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
: m_fitToSlices(new QAction(tr("Fit To Slices"), this))
, m_resolutionUnits(QString("nm"))
, m_activeChannel(nullptr)
, m_activeCategory(nullptr)
, m_viewResolution{1., 1., 1.}
, m_toolGroup{nullptr}
, m_contextualToolBar{nullptr}
{
//   QSettings settings(CESVIMA, ESPINA);
  bool fitEnabled;

//   if (!settings.allKeys().contains(FIT_TO_SLICES))
//   {
//     settings.setValue(FIT_TO_SLICES, true);
//     fitEnabled = true;
//   }
//   else
//     fitEnabled = settings.value(FIT_TO_SLICES, true).toBool();

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
  m_espinaViews << view;
  //connect(view, )
}

//----------------------------------------------------------------------------
void ViewManager::registerView(RenderView* view)
{
  Q_ASSERT(!m_renderViews.contains(view));
  m_renderViews << view;
  Q_ASSERT(!m_espinaViews.contains(view));
  m_espinaViews << view;

  view->setSelector(m_selector);
  view->setColorEngine(m_colorEngine);
}

//----------------------------------------------------------------------------
void ViewManager::registerView(View2D* view)
{
  Q_ASSERT(!m_renderViews.contains(view));
  m_renderViews << view;
  Q_ASSERT(!m_espinaViews.contains(view));
  m_espinaViews << view;
  Q_ASSERT(!m_sliceViews.contains(view));
  m_sliceViews << view;
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
void ViewManager::setSelection(SelectableView::Selection selection)
{
  if (m_selection == selection) return;


  for (int i = 0; i < m_selection.size(); i++)
  {
    m_selection[i]->setSelected(false);
  }

  m_selection = selection;

//   qDebug() << "Selection Changed";
  for (int i = 0; i < m_selection.size(); i++)
  {
    m_selection[i]->setSelected(true);
//     qDebug() << "-" << m_selection[i]->data().toString();
  }

  //TODO 2012-10-07 computeSelectionCenter();

  emit selectionChanged(m_selection);
}

//----------------------------------------------------------------------------
SegmentationAdapterList ViewManager::selectedSegmentations() const
{
  SegmentationAdapterList selection;

  for(auto item: m_selection)
  {
    if (ItemAdapter::Type::SEGMENTATION == item->type())
      selection << segmentationPtr(item);
  }

  return selection;
}

//----------------------------------------------------------------------------
void ViewManager::clearSelection()
{
  if (!m_selection.isEmpty())
  {
    m_selection.clear();

    emit selectionChanged(m_selection);
  }
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

  QAction *separator; // Cheap way of removing last separator without an 'if' in the external loop
  if (m_contextualToolBar)
  {
    for(auto tool : group->tools())
    {
      for(auto action : tool->actions())
      {
        m_contextualToolBar->addAction(action);
      }
      separator = m_contextualToolBar->addSeparator();
    }
    m_contextualToolBar->removeAction(separator);
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
void ViewManager::setSelector(SelectorSPtr selector)
{
  if (m_selector)
  {
    m_selector->setInUse(false);
  }

  m_selector = selector;

  if (m_selector)
    m_selector->setInUse(true);

  for(auto view : m_renderViews)
  {
    view->setSelector(m_selector);
  }
}

//----------------------------------------------------------------------------
void ViewManager::unsetSelector(SelectorSPtr selector)
{
  if (m_selector == selector)
  {
    m_selector.reset();

    for(auto view : m_renderViews)
    {
      view->setSelector(m_selector);
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
void ViewManager::addWidget(EspinaWidget* widget)
{
  widget->setViewManager(this);
  for(auto view: m_renderViews)
    view->addWidget(widget);
}

//----------------------------------------------------------------------------
void ViewManager::removeWidget(EspinaWidget* widget)
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
void ViewManager::setCrosshairVisibility(bool value)
{
  for(auto view: m_sliceViews)
    view->setCrosshairVisibility(value);
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
