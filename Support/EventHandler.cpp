/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2013 Félix de las Pozas Álvarez <felixdelaspozas@gmail.com>

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

#include "EventHandler.h"

using namespace EspINA;

//-----------------------------------------------------------------------------
EventHandler::EventHandler()
: m_inUse(false)
, m_cursor(Qt::CrossCursor)
{
}

//-----------------------------------------------------------------------------
void EventHandler::setInUse(bool value)
{
  if (m_inUse != value)
  {
    m_inUse = value;

    emit eventHandlerInUse(m_inUse);
  }
}

//-----------------------------------------------------------------------------
bool EventHandler::filterEvent(QEvent *e, RenderView *view)
{
  return false;
}





