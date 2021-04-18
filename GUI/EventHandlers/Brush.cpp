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

// ESPINA
#include "Brush.h"
#include "StrokePainter.h"
#include <GUI/View/RenderView.h>

// Qt
#include <QApplication>
#include <QEvent>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>

using namespace ESPINA;

//-----------------------------------------------------------------------------
Brush::Brush()
: m_color      {Qt::red}
, m_borderColor{Qt::blue}
, m_radius     {20}
, m_spacing    {1,1,1}
, m_origin     {0,0,0}
{
  qRegisterMetaType<Stroke>("BrushStroke");

  connect(this, SIGNAL(trackStarted(Track, RenderView*)),
          this, SLOT(onTrackStarted(Track, RenderView*)));

  connect(this, SIGNAL(trackUpdated(Track)),
          this, SLOT(onTrackUpdated(Track)));

  connect(this, SIGNAL(trackStopped(Track,RenderView*)),
          this, SLOT(onTrackStopped(Track,RenderView*)));

  updateCursor();
}

//-----------------------------------------------------------------------------
bool Brush::filterEvent(QEvent *e, RenderView *view)
{
  switch(e->type())
  {
    case QEvent::Wheel:
      {
        if (QApplication::keyboardModifiers().testFlag(Qt::ControlModifier))
        {
          QWheelEvent *we = static_cast<QWheelEvent *>(e);
          int numSteps = we->delta() / 8 / 15;  //Refer to QWheelEvent doc.
          auto newRadius = m_radius - numSteps;
          setRadius(newRadius);
          return true;
        }
      }
      break;
    default:
      break;
  }

  return PointTracker::filterEvent(e, view);
}
//-----------------------------------------------------------------------------
void Brush::setColor(const QColor color)
{
  m_color = color;

  updateCursor();
}

//-----------------------------------------------------------------------------
QColor Brush::color() const
{
  return m_color;
}

//-----------------------------------------------------------------------------
void Brush::setBorderColor(const QColor borderColor)
{
  m_borderColor = borderColor;

  updateCursor();
}

//-----------------------------------------------------------------------------
QColor Brush::borderColor() const
{
  return m_borderColor;
}

//-----------------------------------------------------------------------------
void Brush::setImage(const QImage &image)
{
  m_image = image;

  updateCursor();
}

//-----------------------------------------------------------------------------
void Brush::clearImage()
{
  m_image = QVariant();

  updateCursor();
}

//-----------------------------------------------------------------------------
void Brush::setRadius(const int value)
{
  if(value == m_radius) return;

  if (value < 5)
  {
    m_radius = 5;
  }
  else
  {
    if (value > 40)
    {
      m_radius = 40;
    }
    else
    {
      m_radius = value;
    }
  }

  updateCursor();

  emit radiusChanged(m_radius);
}

//-----------------------------------------------------------------------------
int Brush::radius() const
{
  return m_radius;
}

//-----------------------------------------------------------------------------
void Brush::setOrigin(const NmVector3 &origin)
{
  m_origin = origin;
}

//-----------------------------------------------------------------------------
NmVector3 Brush::origin() const
{
  return m_origin;
}

//-----------------------------------------------------------------------------
void Brush::setSpacing(const NmVector3 &spacing)
{
  m_spacing = spacing;
}

//-----------------------------------------------------------------------------
NmVector3 Brush::spacing() const
{
  return m_spacing;
}

//-----------------------------------------------------------------------------
void Brush::onTrackStarted(PointTracker::Track track, RenderView *view)
{
  configureBrush(view);

  m_stroke.clear();

  emit strokeStarted(view);

  emit strokeUpdated(createStroke(track));

}

//-----------------------------------------------------------------------------
void Brush::onTrackUpdated(PointTracker::Track track)
{
  auto stroke = createStroke(track);

  m_stroke << stroke;

  emit strokeUpdated(stroke);
}

//-----------------------------------------------------------------------------
void Brush::onTrackStopped(PointTracker::Track track, RenderView *view)
{
  emit strokeFinished(m_stroke, view);
}

//-----------------------------------------------------------------------------
Brush::Stroke Brush::createStroke(PointTracker::Track track)
{
  Stroke stroke;

  Nm MIN_DELTA = m_radius/2;

  NmVector3 last = track.first();

  for (int i = 0; i < 3; ++i)
  {
    last[i] += 2*MIN_DELTA;
  }

  for(auto point: track)
  {
    if (fabs(last[0] - point[0]) > MIN_DELTA ||
        fabs(last[1] - point[1]) > MIN_DELTA ||
        fabs(last[2] - point[2]) > MIN_DELTA)
    {
      stroke << createStrokePoint(point);
      last = point;
    }
  }

  return stroke;
}

//-----------------------------------------------------------------------------
void Brush::updateCursor()
{
  int width = 2*m_radius;

  QPixmap pixmap(width, width);
  pixmap.fill(Qt::transparent);

  QPainter painter(&pixmap);
  painter.setBrush(QBrush(m_color));
  painter.setPen(QPen(m_borderColor));
  painter.drawEllipse(0, 0, width-1, width-1);

  if (m_image.isValid())
  {
    auto pos         = m_radius/2;
    auto topLeft     = QPoint(pos, pos);
    auto cursorImage = m_image.value<QImage>();
    if(!cursorImage.isNull())
    {
      auto scaledImage = cursorImage.scaledToWidth(m_radius);
      painter.drawImage(topLeft, scaledImage);
    }
  }

  setCursor(QCursor(pixmap));
}
