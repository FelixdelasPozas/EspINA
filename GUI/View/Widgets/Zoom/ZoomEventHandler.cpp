/*

 Copyright (C) 2015 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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
#include <GUI/View/Widgets/Zoom/ZoomEventHandler.h>

// Qt
#include <QPixmap>
#include <QCursor>
#include <QEvent>
#include <QMouseEvent>

using namespace ESPINA;

//----------------------------------------------------------------------------
ZoomEventHandler::ZoomEventHandler()
: m_isDragging{false}
{
  QPixmap cursorBitmap;

  cursorBitmap.load(":/espina/zoom_cursor.png", "PNG", Qt::ColorOnly);

  setCursor(QCursor(cursorBitmap, 0, 0));
}

//----------------------------------------------------------------------------
ZoomEventHandler::~ZoomEventHandler()
{
}

//----------------------------------------------------------------------------
bool ZoomEventHandler::filterEvent(QEvent* e, RenderView* view)
{
  if (e->type() == QEvent::MouseMove || e->type() == QEvent::MouseButtonPress || e->type() == QEvent::MouseButtonRelease)
  {
    auto me = static_cast<QMouseEvent *>(e);
    auto position = me->pos();

    if (e->type() == QEvent::MouseMove && m_isDragging)
    {
      emit movement(position, view);
      return true;
    }

    if (e->type() == QEvent::MouseButtonPress && me->button() == Qt::LeftButton && !m_isDragging)
    {
      m_isDragging = true;
      emit leftPress(position, view);
      return true;
    }

    if (e->type() == QEvent::MouseButtonRelease && me->button() == Qt::LeftButton && m_isDragging)
    {
      m_isDragging = false;
      emit leftRelease(position, view);
      return true;
    }

    return false;
  }

  return false;
}
