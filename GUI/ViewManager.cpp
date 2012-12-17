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

#include <vtkMath.h>

// Qt
#include <QDebug>

using namespace EspINA;

//----------------------------------------------------------------------------
ViewManager::ViewManager()
: m_voi(NULL)
, m_tool(NULL)
, m_activeChannel(NULL)
, m_activeTaxonomy(NULL)
, m_colorEngine(NULL)
{
}

//----------------------------------------------------------------------------
ViewManager::~ViewManager()
{
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
      selection << qSharedPointerDynamicCast<Segmentation>(item);
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
void ViewManager::setVOI(IVOI *voi)
{
  if (m_voi && m_voi != voi)
    m_voi->setInUse(false);

  m_voi = voi;

  if (m_tool && m_voi)
  {
    m_tool->setInUse(false);
    m_tool = NULL;
  }

  if (m_voi)
    m_voi->setInUse(true);
}

//----------------------------------------------------------------------------
void ViewManager::setActiveTool(ITool* tool)
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
        setVOI(NULL);
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
    m_tool = NULL;
  }
}

//----------------------------------------------------------------------------
void ViewManager::unsetActiveTool(ITool* tool)
{
  if (m_tool == tool)
  {
    m_tool->setInUse(false);
    m_tool = NULL;
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
void ViewManager::updateSegmentationRepresentations(SegmentationList list)
{
  foreach(IEspinaView *view, m_espinaViews)
  {
    view->updateSegmentationRepresentations(list);
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
  updateViews();
}

//----------------------------------------------------------------------------
void ViewManager::focusViewsOn(Nm *center)
{
  foreach(EspinaRenderView *rView, m_renderViews)
    rView->centerViewOn(center, true);
}

