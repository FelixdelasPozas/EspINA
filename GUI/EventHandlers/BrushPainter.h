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

#include <GUI/EspinaGUI_Export.h>

// ESPINA
#include <GUI/EventHandlers/MaskPainter.h>
#include "Brush.h"

namespace ESPINA
{
  /** \class BrushPainter
   * \brief Painter with a brush radius and color.
   *
   */
  class EspinaGUI_EXPORT BrushPainter
  : public MaskPainter
  {
     Q_OBJECT
    public:
      /** \brief BrushPainter class constructor.
       * \param[in] brush brush object.
       *
       */
      explicit BrushPainter(BrushSPtr brush);

      /** \brief Sets if the painter must introduce and remove stroke actor from the view.
       * \param[in] value true to let the painter manage the actor and false otherwise.
       *
       */
      void setManageStrokeActor(bool value);

      /** \brief Returns the painter of this BrushPainter.
       *
       */
      StrokePainterSPtr strokePainter();

      virtual void setColor(const QColor &color) override;

      /** \brief Implements Brush::setBorderColor().
       *
       */
      void setBorderColor(const QColor borderColor);

      /** \brief Implements Brush::borderColor().
       *
       */
      QColor borderColor() const
      { return m_brush->borderColor(); }

      /** \brief Implements Brush::setImage().
       *
       */
      void setImage(const QImage &image);

      /** \brief Implements Brush::clearImage().
       *
       */
      void clearImage();

      /** \brief Implements Brush::setRadius().
       *
       */
      void setRadius(const int value);

      /** \brief Implements Brush::radius().
       *
       */
      int radius() const
      { return m_brush->radius(); }

      void showEraseStrokes(bool value)
      { m_showEraseStrokes = value; }

    signals:
      void strokeStarted(BrushPainter *painter, RenderView *view);

      void radiusChanged(int value);

    private slots:
      void onStrokeStarted(RenderView *view);

      void onStrokeFinished(Brush::Stroke stroke, RenderView *view);

      void onRadiusChanged(int value);

    private:
      virtual void updateCursor(DrawingMode mode) override;

      virtual void onMaskPropertiesChanged(const NmVector3 &spacing, const NmVector3 &origin = NmVector3()) override;


      BinaryMaskSPtr<unsigned char> strokeMask(const Brush::Stroke &stroke,
                                               const NmVector3     &spacing,
                                               const NmVector3     &origin) const;

    private:
      BrushSPtr         m_brush;
      StrokePainterSPtr m_strokePainter;
      bool              m_manageActors;
      bool              m_showEraseStrokes;
      DrawingMode       m_actualStrokeMode;
  };

  using BrushPainterSPtr = std::shared_ptr<BrushPainter>;
}

#endif // ESPINA_BRUSH_PAINTER_H
