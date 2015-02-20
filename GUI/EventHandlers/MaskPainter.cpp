/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2015  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "MaskPainter.h"
#include <QKeyEvent>
#include <QApplication>

using namespace ESPINA;

//------------------------------------------------------------------------
MaskPainter::MaskPainter(PointTrackerSPtr handler)
: m_tracker(handler)
, m_canErase(true)
, m_mode(DrawingMode::PAINTING)
{
}

//------------------------------------------------------------------------
bool MaskPainter::filterEvent(QEvent *e, RenderView *view)
{
  QKeyEvent   *ke = nullptr;

  switch(e->type())
  {
    case QEvent::KeyPress:
    case QEvent::KeyRelease:
      ke = static_cast<QKeyEvent *>(e);
      if ((ke->key() == Qt::Key_Shift) && !m_tracker->isTracking() && m_canErase)
      {
        updateDrawingMode();
        return true;
      }
      break;
    default:
      break;
  }

  return m_tracker->filterEvent(e, view);
}

//------------------------------------------------------------------------
PointTrackerSPtr MaskPainter::pointTracker() const
{
  return m_tracker;
}

//------------------------------------------------------------------------
void MaskPainter::setMaskProperties(const NmVector3 &spacing, const NmVector3 &origin)
{
  m_origin  = origin;
  m_spacing = spacing;

  onMaskPropertiesChanged(spacing, origin);
}

//------------------------------------------------------------------------
void MaskPainter::setCanErase(bool value)
{
  m_canErase = value;
}

//------------------------------------------------------------------------
void MaskPainter::setDrawingMode(const DrawingMode mode)
{
  m_mode = mode;

  updateCursor(m_mode);
}

//------------------------------------------------------------------------
void MaskPainter::setColor(const QColor &color)
{
  m_color = color;

  updateCursor(m_mode);
}

//------------------------------------------------------------------------
void MaskPainter::updateDrawingMode()
{
  auto mode = currentMode();

  updateCursor(mode);

  emit drawingModeChanged(mode);
}

//------------------------------------------------------------------------
bool MaskPainter::ShiftKeyIsDown() const
{
  return QApplication::keyboardModifiers().testFlag(Qt::ShiftModifier);
}

//------------------------------------------------------------------------
DrawingMode MaskPainter::currentMode() const
{
  DrawingMode mode = m_mode;

  if (ShiftKeyIsDown())
  {
    auto isPainting = (m_mode == DrawingMode::PAINTING);

    mode = isPainting?DrawingMode::ERASING:DrawingMode::PAINTING;
  }

  return mode;
}
