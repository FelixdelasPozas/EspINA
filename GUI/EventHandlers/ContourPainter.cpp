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
#include <GUI/View/Widgets/Contour/ContourWidget.h>

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
  , m_widget            {nullptr}
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

    if(!ke || !canErase()) return false;

    switch(e->type())
    {
      case QEvent::KeyPress:
      case QEvent::KeyRelease:
        if(ke->key() == Qt::Key_Shift)
        {
          auto mode = drawingMode();
          auto newMode = (mode == DrawingMode::PAINTING ? DrawingMode::ERASING : DrawingMode::PAINTING);
          setDrawingMode(newMode);
          m_widget->setDrawingMode(newMode);
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
  void ContourPainter::setContourWidget(ContourWidget *widget)
  {
    if(m_widget)
    {
      disconnect(m_widget, SIGNAL(contour(BinaryMaskSPtr<unsigned char>)),
                 this,     SIGNAL(stopPainting(BinaryMaskSPtr<unsigned char>)));
    }

    m_widget = widget;

    if(m_widget)
    {
      connect(m_widget, SIGNAL(contour(BinaryMaskSPtr<unsigned char>)),
              this,     SIGNAL(stopPainting(BinaryMaskSPtr<unsigned char>)));

      updateContourWidget();
    }
  }

  //------------------------------------------------------------------------
  void ContourPainter::setMinimumPointDistance(Nm distance)
  {
    m_minDistance = distance;

    updateContourWidget();
  }

  //------------------------------------------------------------------------
  void ContourPainter::updateCursor(DrawingMode mode)
  {
    updateContourWidget();
  }

  //------------------------------------------------------------------------
  void ContourPainter::onMaskPropertiesChanged(const NmVector3 &spacing, const NmVector3 &origin)
  {
    m_maskSpacing = spacing;

    updateContourWidget();
  }

  //------------------------------------------------------------------------
  void ContourPainter::updateContourWidget()
  {
    if(m_widget)
    {
      m_widget->setDrawingMode(drawingMode());
      m_widget->setPolygonColor(color());
      m_widget->setSpacing(m_maskSpacing);
      m_widget->setMinimumPointDistance(m_minDistance);
    }
  }

} // namespace ESPINA
