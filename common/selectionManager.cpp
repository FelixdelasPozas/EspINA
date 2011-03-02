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

// Debug
#include <QDebug>

//------------------------------------------------------------------------
// SELECTION MANAGER
//------------------------------------------------------------------------
SelectionManager *SelectionManager::m_singleton = new SelectionManager();

SelectionManager::SelectionManager()
  : QObject()
  , m_sh(NULL)
{
}

//------------------------------------------------------------------------
void SelectionManager::setSelectionHandler(ISelectionHandler* sh)
{
  if (m_sh && m_sh != sh)
    m_sh->abortSelection();
  m_sh = sh;
}

//------------------------------------------------------------------------
void SelectionManager::pointSelected(const Point coord)
{
  //TODO: Create selection from coord information
  Selection sel;
  sel.coord = coord;
  if (m_sh)
  {
    m_sh->handle(sel);
    //qDebug() << "Selection managed";
  }
  //else
    //qDebug() << "Selection ignored";
}