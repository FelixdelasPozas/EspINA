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
#include "BrushPainter.h"
#include <GUI/View/View2D.h>
#include <vtkImplicitFunction.h>

using namespace ESPINA;

//-----------------------------------------------------------------------------
BrushPainter::BrushPainter(BrushSPtr brush)
: MaskPainter       {brush}
, m_brush           {brush}
, m_showStroke      {true}
, m_actualStrokeMode{DrawingMode::PAINTING}
{
  setCursor(m_brush->cursor());
  m_brush->setInterpolation(true);

  connect(m_brush.get(), SIGNAL(strokeStarted(RenderView*)),
          this,          SLOT(onStrokeStarted(RenderView*)));
  
  connect(m_brush.get(), SIGNAL(strokeFinished(Brush::Stroke,RenderView*)),
          this,          SLOT(onStrokeFinished(Brush::Stroke,RenderView*)));

  connect(m_brush.get(), SIGNAL(radiusChanged(int)),
          this,          SLOT(onRadiusChanged(int)));
}

//-----------------------------------------------------------------------------
void BrushPainter::setStrokeVisibility(bool value)
{
  m_showStroke = value;
}

//-----------------------------------------------------------------------------
StrokePainterSPtr BrushPainter::strokePainter()
{
  return m_strokePainter;
}

//-----------------------------------------------------------------------------
void BrushPainter::setColor(const QColor &color)
{
  MaskPainter::setColor(color);

  m_brush->setColor(color);
}

//-----------------------------------------------------------------------------
void BrushPainter::onStrokeStarted(RenderView *view)
{
  m_strokePainter = std::make_shared<StrokePainter>(m_spacing, m_origin, view,
                                                    currentMode(), m_brush.get());

  m_brush->setMaximumPointDistance(0.25 * radius() * view2D_cast(view)->scale());
  m_actualStrokeMode = currentMode();

  emit strokeStarted(this, view);

  if (m_showStroke)
  {
    view->addActor(m_strokePainter->strokeActor());
  }

}

//-----------------------------------------------------------------------------
void BrushPainter::onStrokeFinished(Brush::Stroke stroke, RenderView *view)
{
  if (m_showStroke)
  {
    view->removeActor(m_strokePainter->strokeActor());
  }

  m_strokePainter.reset();

  emit stopPainting(strokeMask(stroke, m_brush->spacing(), m_brush->origin()));
}

//-----------------------------------------------------------------------------
void BrushPainter::onRadiusChanged(int value)
{
  setCursor(m_brush->cursor());

  emit radiusChanged(value);
}

//-----------------------------------------------------------------------------
void BrushPainter::updateCursor(DrawingMode mode)
{
  m_brush->setColor(m_color);

  if (DrawingMode::PAINTING == mode)
  {
    m_brush->setBorderColor(Qt::blue);
  }
  else
  {
    m_brush->setBorderColor(Qt::red);
  }

  setCursor(m_brush->cursor());
}

//-----------------------------------------------------------------------------
void BrushPainter::onMaskPropertiesChanged(const NmVector3 &spacing, const NmVector3 &origin)
{
  m_brush->setOrigin(origin);
  m_brush->setSpacing(spacing);
}

//------------------------------------------------------------------------
BinaryMaskSPtr<unsigned char> BrushPainter::strokeMask(const Brush::Stroke &stroke,
                                                      const NmVector3 &spacing,
                                                      const NmVector3 &origin) const
{
  Q_ASSERT(!stroke.isEmpty());

  Bounds strokeBounds;

  for (auto strokePoint : stroke)
  {
    auto strokePointBounds = strokePoint.second;

    if (!strokeBounds.areValid())
    {
      strokeBounds = strokePointBounds;
    }
    else
    {
      strokeBounds = boundingBox(strokeBounds, strokePointBounds);
    }
  }

  VolumeBounds maskBounds(strokeBounds, spacing, origin);

  auto mask = std::make_shared<BinaryMask<unsigned char>>(maskBounds.bounds(), spacing, origin);

  for (auto strokePoint : stroke)
  {
    BinaryMask<unsigned char>::region_iterator it(mask.get(), strokePoint.second);
    while (!it.isAtEnd())
    {
      auto index = it.getIndex();
      if (strokePoint.first->FunctionValue(index.x * spacing[0], index.y * spacing[1], index.z * spacing[2]) <= 0)
      {
        it.Set();
      }
      ++it;
    }
  }

  auto value = (m_actualStrokeMode == DrawingMode::ERASING) ? SEG_BG_VALUE : SEG_VOXEL_VALUE;
  mask->setForegroundValue(value);

  return mask;
}
