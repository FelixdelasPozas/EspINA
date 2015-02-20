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

#ifndef ESPINA_BRUSH_H
#define ESPINA_BRUSH_H

#include "PointTracker.h"
#include <Core/Utils/Bounds.h>

#include <QColor>
#include <QImage>
#include <vtkSmartPointer.h>

class vtkImplicitFunction;

namespace ESPINA
{
  class StrokePainter;

  class Brush
  : public PointTracker
  {
    Q_OBJECT
  public:
    using StrokePoint = QPair<vtkSmartPointer<vtkImplicitFunction>, Bounds>;
    using Stroke      = QList<StrokePoint>;

  public:
    explicit Brush();

    void setColor(const QColor color);

    QColor color() const;

    void setBorderColor(const QColor borderColor);

    QColor borderColor() const;

    void setImage(const QImage &image);

    void clearImage();

    void setRadius(const int value);

    int radius() const;


//     void showStroke(spacing, origin);
//     void hideStroke();
    void setSpacing(const NmVector3 &spacing);

    NmVector3 spacing() const;

    void setOrigin(const NmVector3 &origin);

    NmVector3 origin() const;

    void setStrokeVisibility(bool visible);

  signals:
    void strokeStarted(Brush::Stroke stroke, RenderView *view);
    void strokeUpdated(Brush::Stroke stroke);
    void strokeFinished(Brush::Stroke stroke, RenderView *view);

  private slots:
    void onTrackStarted(Track track, RenderView *view);
    void onTrackUpdated(Track track);
    void onTrackStopped(Track track, RenderView *view);

  private:
    /** \brief
     *
     *
     */
    Stroke createStroke(Track track);

    /** \brief Returns the StrokePoint created for track point.
     * \param[in] point track point.
     *
     */
    virtual StrokePoint createStrokePoint(NmVector3 point) = 0;

    virtual void configureBrush(RenderView *view) = 0;

    void updateCursor();

  private:
    QColor    m_color;
    QColor    m_borderColor;
    int       m_radius;

    QVariant  m_image;

    NmVector3 m_spacing;
    NmVector3 m_origin;
    bool      m_showStroke;

    std::shared_ptr<StrokePainter> m_strokePainter;
  };

  using BrushSPtr = std::shared_ptr<Brush>;
}

#endif // ESPINA_BRUSH_H
