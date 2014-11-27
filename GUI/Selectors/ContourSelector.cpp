/*

    Copyright (C) 2014 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

    This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
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

// ESPINA
#include "ContourSelector.h"

// Qt
#include <QPolygon>
#include <QEvent>
#include <QMouseEvent>
#include <QApplication>
#include <QVTKWidget.h>

using namespace ESPINA;

//-----------------------------------------------------------------------------
bool ContourSelector::filterEvent(QEvent* e, RenderView *view)
{
  switch (e->type())
  {
    case QEvent::Enter:
      return Selector::filterEvent(e, view);
      break;
    case QEvent::Leave:
      return Selector::filterEvent(e, view);
      break;
    case QEvent::Wheel:
      return Selector::filterEvent(e, view);
      break;
    default:
      break;
  }

  return true;
}

//-----------------------------------------------------------------------------
QCursor ContourSelector::cursor()
{
  return m_cursor;
}
