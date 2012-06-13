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
void SelectionManager::setSelection(SelectionHandler::MultiSelection sel) const
{
  if (m_handler)
    m_handler->setSelection(sel);
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
    QApplication::restoreOverrideCursor();
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
