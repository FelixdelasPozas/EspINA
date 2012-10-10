/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) <year>  <name of author>

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

#include "SelectionManager.h"

#include "EspinaRegions.h"
#include "RectangularSelection.h"
#include "model/Segmentation.h"

#include <QApplication>

SelectionManager *SelectionManager::m_singleton = NULL;//new SelectionManager();

SelectionManager* SelectionManager::instance()
{
  if (!m_singleton)
    m_singleton = new SelectionManager();

  return m_singleton;
}

SelectionManager::SelectionManager()
: m_handler(NULL)
, m_voi(NULL)
{
  memset(m_selectionCenter, 0, 3*sizeof(Nm));
}

//------------------------------------------------------------------------
bool SelectionManager::filterEvent(QEvent* e, EspinaRenderView* view) const
{
  bool res = false;
  if (m_handler)
    res = m_handler->filterEvent(e, view);

  return res;
}

//------------------------------------------------------------------------
void SelectionManager::setSelection(Selection selection)
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

  computeSelectionCenter();
  //TODO 2012-10-04: EspinaCore::instance()->viewManger()->currentView()->forceRender();
  emit selectionChanged(m_selection);
}

//------------------------------------------------------------------------
QCursor SelectionManager::cursor() const
{
  if (m_handler)
    return m_handler->cursor();
  else
    return QCursor(Qt::ArrowCursor);
}

//------------------------------------------------------------------------
void SelectionManager::computeSelectionCenter()
{
  memset(m_selectionCenter, 0, 3*sizeof(Nm));

  for (int i = 0; i < m_selection.size(); i++)
  {
    PickableItem *item = m_selection[i];
    if (ModelItem::SEGMENTATION == item->type())
    {
      Segmentation *seg = dynamic_cast<Segmentation *>(item);
      double bounds[6];
      VolumeBounds(seg->itkVolume(), bounds);
      m_selectionCenter[0] += bounds[0] + (bounds[1]-bounds[0])/2.0;
      m_selectionCenter[1] += bounds[2] + (bounds[3]-bounds[2])/2.0;
      m_selectionCenter[2] += bounds[4] + (bounds[5]-bounds[4])/2.0;
    }
  }
  for (int i = 0; i < 3; i++)
    m_selectionCenter[i] /= m_selection.size();
}

//------------------------------------------------------------------------
void SelectionManager::setSelectionHandler(IPicker* sh)
{
  if (m_handler && m_handler != sh)
    m_handler->abortSelection();

  m_handler = sh;

  if (m_voi)
  {
    m_voi->setEnabled(!m_handler);
  }
}

//------------------------------------------------------------------------
void SelectionManager::unsetSelectionHandler(IPicker* sh)
{
  if (m_handler == sh)
  {
    m_handler = NULL;
  }
}

//------------------------------------------------------------------------
void SelectionManager::setVOI(EspinaWidget *voi)
{
//   if (m_voi)
//     m_voi->cancel();

  m_voi = voi;

  if (m_handler && m_voi)
  {
    m_handler->abortSelection();
    m_handler = NULL;
  }
}
