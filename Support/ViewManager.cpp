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

// EspINA
#include <Core/ColorEngines/IColorEngine.h>
#include <Core/Model/PickableItem.h>
#include <Core/Model/Segmentation.h>
#include <GUI/QtWidget/SliceView.h>
#include <GUI/Tools/IVOI.h>
#include <Core/EspinaSettings.h>
#include <Core/Utils/Measure.h>

#include <vtkMath.h>

// Qt
#include <QSettings>
#include <QString>
#include <QStack>
#include <QDebug>

using namespace EspINA;

const QString FIT_TO_SLICES ("ViewManager::FitToSlices");

//----------------------------------------------------------------------------
ViewManager::ViewManager()
: m_fitToSlices(new QAction(tr("Fit To Slices"), this))
, m_resolutionUnits(QString("nm"))
, m_activeChannel(NULL)
, m_activeTaxonomy(NULL)
{
  QSettings settings(CESVIMA, ESPINA);
  bool fitEnabled;

  if (!settings.allKeys().contains(FIT_TO_SLICES))
  {
    settings.setValue(FIT_TO_SLICES, true);
    fitEnabled = true;
  }
  else
    fitEnabled = settings.value(FIT_TO_SLICES, true).toBool();

  m_fitToSlices->setCheckable(true);
  m_fitToSlices->setChecked(fitEnabled);

  connect(m_fitToSlices, SIGNAL(toggled(bool)), this, SLOT(toggleFitToSlices(bool)));
  m_viewResolution[0] = m_viewResolution[1] = m_viewResolution[2] = 1.0;
}

//----------------------------------------------------------------------------
ViewManager::~ViewManager()
{
//   qDebug() << "********************************************************";
//   qDebug() << "              Destroying View Manager";
//   qDebug() << "********************************************************";
}

//----------------------------------------------------------------------------
void ViewManager::registerView(IEspinaView* view)
{
  Q_ASSERT(!m_espinaViews.contains(view));
  m_espinaViews << view;
}

//----------------------------------------------------------------------------
void ViewManager::registerView(EspinaRenderView* view)
{
  Q_ASSERT(!m_renderViews.contains(view));
  m_renderViews << view;
  Q_ASSERT(!m_espinaViews.contains(view));
  m_espinaViews << view;
}

//----------------------------------------------------------------------------
void ViewManager::registerView(SliceView* view)
{
  Q_ASSERT(!m_renderViews.contains(view));
  m_renderViews << view;
  Q_ASSERT(!m_espinaViews.contains(view));
  m_espinaViews << view;
  Q_ASSERT(!m_sliceViews.contains(view));
  m_sliceViews << view;
}

//----------------------------------------------------------------------------
void ViewManager::unregisterView(IEspinaView* view)
{
  Q_ASSERT(m_espinaViews.contains(view));
  m_espinaViews.removeAll(view);
}

//----------------------------------------------------------------------------
void ViewManager::unregisterView(EspinaRenderView* view)
{
  Q_ASSERT(m_renderViews.contains(view));
  m_renderViews.removeAll(view);
  Q_ASSERT(m_espinaViews.contains(view));
  m_espinaViews.removeAll(view);
}

//----------------------------------------------------------------------------
void ViewManager::unregisterView(SliceView* view)
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
  foreach(EspinaRenderView *view, m_renderViews)
    view->setSelectionEnabled(enable);
}

//----------------------------------------------------------------------------
void ViewManager::setSelection(ViewManager::Selection selection)
{
  if (m_selection == selection)
    return;

  for (int i = 0; i < m_selection.size(); i++)
    m_selection[i]->setSelected(false);

  m_selection = selection;

//   qDebug() << "Selection Changed";
  for (int i = 0; i < m_selection.size(); i++)
  {
    m_selection[i]->setSelected(true);
//     qDebug() << "-" << m_selection[i]->data().toString();
  }

  //TODO 2012-10-07 computeSelectionCenter();

  emit selectionChanged(m_selection, true);
}

//----------------------------------------------------------------------------
SegmentationList ViewManager::selectedSegmentations() const
{
  SegmentationList selection;

  foreach(PickableItemPtr item, m_selection)
  {
    if (EspINA::SEGMENTATION == item->type())
      selection << segmentationPtr(item);
  }

  return selection;
}

//----------------------------------------------------------------------------
void ViewManager::clearSelection(bool notifyViews)
{
  if (!m_selection.isEmpty())
  {
    m_selection.clear();

    emit selectionChanged(m_selection, notifyViews);
  }
}

//----------------------------------------------------------------------------
void ViewManager::setVOI(IVOISPtr voi)
{
  if (m_voi && m_voi != voi)
    m_voi->setInUse(false);

  m_voi = voi;

  if (m_tool && m_voi)
  {
    m_tool->setInUse(false);
    m_tool.reset();
  }

  if (m_voi)
    m_voi->setInUse(true);
}

//----------------------------------------------------------------------------
void ViewManager::unsetActiveVOI()
{
  if (m_voi)
  {
    m_voi->setEnabled(false);
    m_voi->setInUse(false);
    m_voi.reset();
  }
}


//----------------------------------------------------------------------------
void ViewManager::setActiveTool(IToolSPtr tool)
{
  Q_ASSERT(tool);

  if (m_tool && m_tool != tool)
    m_tool->setInUse(false);

  m_tool = tool;

  if (m_tool)
  {
    if (m_voi)
    {
      double *voiBounds = m_voi->region();
      if (!vtkMath::AreBoundsInitialized(voiBounds))
        unsetActiveVOI();
        //setVOI(IVOISPtr());
    }
    m_tool->setInUse(true);
  }
}

//----------------------------------------------------------------------------
void ViewManager::unsetActiveTool()
{
  if (m_tool)
  {
    m_tool->setInUse(false);
    m_tool.reset();
  }
}

//----------------------------------------------------------------------------
void ViewManager::unsetActiveTool(IToolSPtr tool)
{
  if (m_tool == tool)
  {
    m_tool->setInUse(false);
    m_tool.reset();
  }
}

//----------------------------------------------------------------------------
bool ViewManager::filterEvent(QEvent* e, EspinaRenderView* view)
{
  bool res = false;

  if (m_voi)
    res = m_voi->filterEvent(e, view);

  if (res && m_tool)
    m_tool->lostEvent(view);

  if (!res && m_tool)
    res = m_tool->filterEvent(e, view);

  return res;
}

//----------------------------------------------------------------------------
QCursor ViewManager::cursor() const
{
  QCursor activeCursor(Qt::ArrowCursor);

  if (m_tool)
    activeCursor = m_tool->cursor();
  else if (m_voi && m_voi->enabled())
    activeCursor = m_voi->cursor();

  return activeCursor;
}

//----------------------------------------------------------------------------
void ViewManager::updateViews()
{
  foreach(EspinaRenderView *view, m_renderViews)
  {
    view->updateView();
  }
}

//----------------------------------------------------------------------------
void ViewManager::toggleFitToSlices(bool enabled)
{
  QSettings settings(CESVIMA, ESPINA);
  settings.setValue(FIT_TO_SLICES, enabled);
  settings.sync();
}

//----------------------------------------------------------------------------
void ViewManager::setActiveChannel(ChannelPtr channel)
{
  m_activeChannel = channel;
  emit activeChannelChanged(channel);
}
//----------------------------------------------------------------------------
void ViewManager::addWidget(EspinaWidget* widget)
{
  widget->setViewManager(this);
  foreach(EspinaRenderView *rView, m_renderViews)
  {
    rView->addWidget(widget);
  }
}

//----------------------------------------------------------------------------
void ViewManager::removeWidget(EspinaWidget* widget)
{
  foreach(EspinaRenderView *rView, m_renderViews)
  {
    rView->removeWidget(widget);
  }
}

//----------------------------------------------------------------------------
void ViewManager::resetViewCameras()
{
  foreach(EspinaRenderView *view, m_renderViews)
  {
    view->resetCamera();
  }
}

//----------------------------------------------------------------------------
void ViewManager::updateSegmentationRepresentations(SegmentationPtr segmentation)
{
  QStack<SegmentationPtr> itemsStack;
  SegmentationList segList;

  itemsStack.push_front(segmentation);
  segList << segmentation;

  while (!itemsStack.isEmpty())
  {
    SegmentationPtr seg = itemsStack.pop();
    foreach(ModelItemSPtr item, seg->relatedItems(EspINA::RELATION_OUT))
      if (item->type() == SEGMENTATION)
      {
        SegmentationPtr relatedSeg = segmentationPtr(item.get());
        if (relatedSeg->isInputSegmentationDependent() && !segList.contains(relatedSeg))
        {
          itemsStack.push_front(relatedSeg);
          segList << relatedSeg;
        }
      }
  }

  foreach(IEspinaView *view, m_espinaViews)
  {
    view->updateSegmentationRepresentations(segList);
  }
}

//----------------------------------------------------------------------------
void ViewManager::updateSegmentationRepresentations(SegmentationList list)
{
  QStack<SegmentationPtr> itemsStack;
  SegmentationList segList;

  foreach(SegmentationPtr seg, list)
  {
    if (segList.contains(seg))
      continue;

    itemsStack.push_front(seg);
    segList << seg;

    while (!itemsStack.isEmpty())
    {
      SegmentationPtr segmentation = itemsStack.pop();
      foreach(ModelItemSPtr item, segmentation->relatedItems(EspINA::RELATION_OUT))
        if (item->type() == SEGMENTATION)
        {
          SegmentationPtr relatedSeg = segmentationPtr(item.get());
          if (relatedSeg->isInputSegmentationDependent() && !segList.contains(relatedSeg))
          {
            itemsStack.push_front(relatedSeg);
            segList << relatedSeg;
          }
        }
    }
  }

  foreach(IEspinaView *view, m_espinaViews)
  {
    view->updateSegmentationRepresentations(segList);
  }
}

//----------------------------------------------------------------------------
void ViewManager::updateChannelRepresentations(ChannelList list)
{
  foreach(IEspinaView *view, m_espinaViews)
  {
    view->updateChannelRepresentations(list);
  }
}

//----------------------------------------------------------------------------
void ViewManager::showCrosshair(bool value)
{
  foreach(EspinaRenderView *rView, m_renderViews)
    rView->showCrosshairs(value);
}

//----------------------------------------------------------------------------
void ViewManager::addSliceSelectors(SliceSelectorWidget* widget,
                                    ViewManager::SliceSelectors selectors)
{
  foreach(SliceView *view, m_sliceViews)
    view->addSliceSelectors(widget, selectors);
}

//----------------------------------------------------------------------------
void ViewManager::removeSliceSelectors(SliceSelectorWidget* widget)
{
  foreach(SliceView *view, m_sliceViews)
    view->removeSliceSelectors(widget);
}


//----------------------------------------------------------------------------
QColor ViewManager::color(SegmentationPtr seg)
{
  QColor segColor(Qt::blue);
  if (m_colorEngine)
    segColor = m_colorEngine->color(seg);

  return segColor;
}

//----------------------------------------------------------------------------
LUTPtr ViewManager::lut(SegmentationPtr seg)
{
  // Get (or create if it doesn't exit) the lut for the segmentations' images
  if (m_colorEngine)
    return m_colorEngine->lut(seg);

  double alpha = 0.8;
  QColor c(Qt::blue);
  seg_lut = vtkLookupTable::New();
  seg_lut->Allocate();
  seg_lut->SetNumberOfTableValues(2);
  seg_lut->Build();
  seg_lut->SetTableValue(0, 0.0, 0.0, 0.0, 0.0);
  seg_lut->SetTableValue(1, c.redF(), c.greenF(), c.blueF(), alpha);
  seg_lut->Modified();

  return seg_lut;
}

//----------------------------------------------------------------------------
void ViewManager::setColorEngine(ColorEngine* engine)
{
  m_colorEngine = engine;
  updateSegmentationRepresentations();
}

//----------------------------------------------------------------------------
void ViewManager::focusViewsOn(Nm *center)
{
  foreach(EspinaRenderView *rView, m_renderViews)
    rView->centerViewOn(center, true);
}

//----------------------------------------------------------------------------
Nm *ViewManager::viewResolution()
{
  if (m_renderViews.empty())
    return NULL;

  // ASSERT: the first view is not from type VOLUME and all the views have to
  // have the same view resolution, so we can get the correct resolution from
  // any of them
  memcpy(m_viewResolution, m_renderViews.first()->sceneResolution(), sizeof(Nm)*3);

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