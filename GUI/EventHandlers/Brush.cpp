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

#include "Brush.h"
#include "StrokePainter.h"
#include <GUI/View/RenderView.h>

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
, m_showStroke {true}
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
  m_radius = value;

  updateCursor();
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
void Brush::setStrokeVisibility(bool visible)
{
  m_showStroke = visible;
}

//-----------------------------------------------------------------------------
void Brush::onTrackStarted(PointTracker::Track track, RenderView *view)
{
  configureBrush(view);

  emit strokeStarted(createStroke(track), view);

  if (m_showStroke)
  {
    m_strokePainter = std::make_shared<StrokePainter>(m_spacing, m_origin, view, this);

    view->addActor(m_strokePainter->strokeActor());
  }

}

//-----------------------------------------------------------------------------
void Brush::onTrackUpdated(PointTracker::Track track)
{
  emit strokeUpdated(createStroke(track));
}

//-----------------------------------------------------------------------------
void Brush::onTrackStopped(PointTracker::Track track, RenderView *view)
{
  if(m_showStroke)
  {
    view->removeActor(m_strokePainter->strokeActor());

    m_strokePainter.reset();
  }

  emit strokeFinished(createStroke(track), view);
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
    if (abs(last[0] - point[0]) > MIN_DELTA
     || abs(last[1] - point[1]) > MIN_DELTA
     || abs(last[2] - point[2]) > MIN_DELTA)
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
    auto pos     = m_radius/2;
    auto topLeft = QPoint(pos, pos);
    auto image   = m_image.value<QImage>().scaledToWidth(m_radius);

    painter.drawImage(topLeft, image);
  }

  setCursor(QCursor(pixmap));
}
