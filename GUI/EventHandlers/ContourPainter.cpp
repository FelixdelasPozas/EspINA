/*

 Copyright (C) 2015 Félix de las Pozas Álvarez <fpozas@cesvima.upm.es>

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
#include <GUI/EventHandlers/ContourPainter.h>

// Qt
#include <QKeyEvent>
#include <QApplication>

namespace ESPINA
{
  //------------------------------------------------------------------------
  ContourPainter::ContourPainter()
  : MaskPainter         {std::make_shared<PointTracker>()}
  , m_minDistance       {40}
  , m_tracking          {false}
  {
    setCursor(Qt::CrossCursor);
    setCanErase(false);
  }

  //------------------------------------------------------------------------
  bool ContourPainter::filterEvent(QEvent *e, RenderView *view)
  {
    // almost all the events are handled by the widget so we let the event pass to the
    // vtk interactor class.
    auto ke = dynamic_cast<QKeyEvent *>(e);

    if(!ke) return false;

    switch(e->type())
    {
      case QEvent::KeyPress:
        if (ke->key() == Qt::Key_Control)
        {
          emit rasterize();
          return true;
        }
        // no break
      case QEvent::KeyRelease:
        if(ke->key() == Qt::Key_Shift)
        {
          if(!canErase()) return false;

          auto mode = drawingMode();
          auto newMode = (mode == DrawingMode::PAINTING ? DrawingMode::ERASING : DrawingMode::PAINTING);
          setDrawingMode(newMode);
          emit drawingModeChanged(newMode);
          return true;
        }
        break;
      default:
        break;
    }

    return false;
  }

  //------------------------------------------------------------------------
  void ContourPainter::setMinimumPointDistance(Nm distance)
  {
    m_minDistance = distance;

    updateWidgetsValues();
  }

  //------------------------------------------------------------------------
  void ContourPainter::updateWidgetsValues() const
  {
    emit drawingModeChanged(drawingMode());
    emit configure(m_minDistance, color(), m_maskSpacing);
  }

  //------------------------------------------------------------------------
  void ContourPainter::updateCursor(DrawingMode mode)
  {
    updateWidgetsValues();
  }

  //------------------------------------------------------------------------
  void ContourPainter::onMaskPropertiesChanged(const NmVector3 &spacing, const NmVector3 &origin)
  {
    m_maskSpacing = spacing;

    updateWidgetsValues();
  }

  //------------------------------------------------------------------------
  void ContourPainter::rasterizeContours()
  {
    emit rasterize();
  }

} // namespace ESPINA
