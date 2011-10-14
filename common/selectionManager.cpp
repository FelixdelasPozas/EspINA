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

#include "selectionManager.h"

#include <pqView.h>
#include "espina.h"

// Debug
#include <QDebug>


//------------------------------------------------------------------------
// SELECTION HANDLER
//------------------------------------------------------------------------
void ISelectionHandler::setSelection(ISelectionHandler::MultiSelection msel)
{
  qDebug("Selection Changed");
  emit selectionChanged(msel);
}

void ISelectionHandler::abortSelection()
{
  qDebug("Selection Aborted");
  emit selectionAborted();
}


//------------------------------------------------------------------------
// SELECTION MANAGER
//------------------------------------------------------------------------
SelectionManager *SelectionManager::m_singleton = new SelectionManager();

SelectionManager::SelectionManager()
  : QObject()
  , m_handler(NULL)
  , m_voi(NULL)
{
}

//------------------------------------------------------------------------
void SelectionManager::setSelectionHandler(ISelectionHandler* sh, QCursor cursor)
{
  if (m_handler && m_handler != sh)
    m_handler->abortSelection();
  m_handler = sh;
  m_handlerCursor = cursor;
  if (m_voi)
  {
    m_voi->setEnabled(!m_handler);
  }
}

//------------------------------------------------------------------------
void SelectionManager::unsetSelectionHandler(ISelectionHandler* sh)
{
  assert(m_handler == sh);
  
  m_handler = NULL;
}

//------------------------------------------------------------------------
void SelectionManager::setVOI(IVOI* voi)
{
  if (m_voi && m_voi != voi)
    m_voi->cancelVOI();
  m_voi = voi;
  if (m_voi)
    m_voi->setSource(EspINA::instance()->activeSample());
  emit VOIChanged(m_voi);
  
  if (m_handler && m_voi)
  {
    m_handler->abortSelection();
    m_handler = NULL;
  }
}

//------------------------------------------------------------------------
// IFilter* SelectionManager::applyVOI(vtkProduct* product)
// {
//   if (m_voi)
//   {
//     //return m_voi->applyVOI(product);    
//   }
//   else
//     return product;
// }
// 
// //------------------------------------------------------------------------
// Product* SelectionManager::restoreVOITransformation(Product* product)
// {
//   if (m_voi)
//   {
//     //return m_voi->restoreVOITransormation(product);
//   }
//   else
//     return product;
// }
