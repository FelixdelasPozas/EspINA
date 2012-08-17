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

#include <QApplication>

#include <RectangularSelection.h>
#include <model/Segmentation.h>


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
, m_activeChannel(NULL)
, m_activeTaxonomy(NULL)
{
  memset(m_selectionCenter, 0, 3*sizeof(Nm));
}

//------------------------------------------------------------------------
bool SelectionManager::filterEvent(QEvent* e, SelectableView* view) const
{
  bool res = false;
  if (m_handler)
    res = m_handler->filterEvent(e, view);

  return res;
}

//------------------------------------------------------------------------
void SelectionManager::setSelection(SelectionHandler::MultiSelection sel)
{
  m_selection = sel;
  computeSelectionCenter();
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
    SelectableItem *item = m_selection[i].second;
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
void SelectionManager::setSelectionHandler(SelectionHandler* sh)
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
void SelectionManager::unsetSelectionHandler(SelectionHandler* sh)
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
