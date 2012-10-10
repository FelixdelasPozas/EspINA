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
#include "common/gui/IEspinaView.h"
#include "ColorEngine.h"
#include "EspinaRenderView.h"
#include <selection/SelectableItem.h>
#include <selection/SelectionHandler.h>

// Qt
#include <QDebug>

//----------------------------------------------------------------------------
ViewManager::ViewManager()
: m_picker(NULL)
, m_activeChannel(NULL)
, m_activeTaxonomy(NULL)
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

  emit selectionChanged(m_selection);
}

//----------------------------------------------------------------------------
void ViewManager::setPicker(IPicker* picker)
{
  if (m_picker && m_picker != picker)
    m_picker->abortPick();

  m_picker = picker;

  if (m_picker)
    foreach(EspinaRenderView *rView, m_renderViews)
    {
      rView->setCursor(m_picker->cursor());
    }

  /*TODO 2012-10-07
   * if (m_voi)
   * {
   *   m_voi->setEnabled(!m_handler);
   }
   */
}

//----------------------------------------------------------------------------
void ViewManager::unsetPicker(IPicker* picker)
{
  if (m_picker == picker)
    m_picker = NULL;

  foreach(EspinaRenderView *rView, m_renderViews)
  {
    rView->setCursor(Qt::ArrowCursor);
  }
}

//----------------------------------------------------------------------------
bool ViewManager::filterEvent(QEvent* e, EspinaRenderView* view) const
{
  bool res = false;
  if (m_picker)
    res = m_picker->filterEvent(e, view);

  return res;
}

//----------------------------------------------------------------------------
QCursor ViewManager::cursor() const
{
  if (m_picker)
    return m_picker->cursor();
  else
    return QCursor(Qt::ArrowCursor);
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
void ViewManager::addWidget(EspinaWidget* widget)
{
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
QColor ViewManager::color(const Segmentation* seg)
{
  QColor segColor(Qt::blue);
  if (!m_engines.isEmpty())
    segColor = m_engines.keys().first()->color(seg);

  return segColor;
}

//----------------------------------------------------------------------------
vtkSmartPointer<vtkLookupTable> ViewManager::lut(const Segmentation* seg)
{
  // Get (or create if it doesn't exit) the lut for the segmentations' images
  double alpha = 0.8;
  QColor c = color(seg);
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
void ViewManager::registerColorEngine(ColorEngine* engine)
{
  Q_ASSERT(!m_engines.contains(engine));
  m_engines.insert(engine, false);
}

//----------------------------------------------------------------------------
void ViewManager::setEnabled(ColorEngine* engine, bool enable)
{
  Q_ASSERT(m_engines.contains(engine));
  m_engines[engine] = enable;
}

//----------------------------------------------------------------------------
void ViewManager::unregisterColorEngine(ColorEngine* engine)
{
  Q_ASSERT(m_engines.contains(engine));
  m_engines.remove(engine);
}
