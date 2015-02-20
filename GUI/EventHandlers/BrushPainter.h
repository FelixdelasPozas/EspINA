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

#ifndef ESPINA_BRUSH_PAINTER_H
#define ESPINA_BRUSH_PAINTER_H

#include <GUI/EventHandlers/MaskPainter.h>
#include "Brush.h"

namespace ESPINA
{
  class BrushPainter
  : public MaskPainter
  {
   Q_OBJECT

  public:
    explicit BrushPainter(BrushSPtr brush);

    BrushSPtr brush();

  signals:
    void strokeStarted(BrushSPtr brush, RenderView *view);

  private slots:
    void onStrokeStarted(Brush::Stroke stroke, RenderView *view);

    void onStrokeFinished(Brush::Stroke stroke, RenderView *view);

  private:
    virtual void updateCursor(MaskPainter::DrawingMode mode) override;

    virtual void onMaskPropertiesChanged(const NmVector3 &spacing, const NmVector3 &origin = NmVector3());


    BinaryMaskSPtr<unsigned char> strokeMask(const Brush::Stroke &stroke,
                                             const NmVector3     &spacing,
                                             const NmVector3     &origin) const;

  private:
    BrushSPtr m_brush;

  };

  using BrushPainterSPtr = std::shared_ptr<BrushPainter>;
}

#endif // ESPINA_BRUSH_PAINTER_H
