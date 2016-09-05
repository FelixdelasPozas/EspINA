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

#include <GUI/EspinaGUI_Export.h>

// ESPINA
#include "PointTracker.h"
#include <Core/Utils/Bounds.h>

// Qt
#include <QColor>
#include <QImage>

// VTK
#include <vtkSmartPointer.h>

class vtkImplicitFunction;

namespace ESPINA
{
  /** \class Brush
   *  \brief Implements a brush with color, radius and origin.
   */
  class EspinaGUI_EXPORT Brush
  : public PointTracker
  {
    Q_OBJECT
  public:
    using StrokePoint = QPair<vtkSmartPointer<vtkImplicitFunction>, Bounds>;
    using Stroke      = QList<StrokePoint>;

  public:
    /** \brief Brush class constructor.
     *
     */
    explicit Brush();

    /** \brief Brush class virtual destructor.
     *
     */
    virtual ~Brush()
    {};

    virtual bool filterEvent(QEvent *e, RenderView *view = nullptr) override;

    /** \brief Sets the color of the brush cursor.
     * \param[in] color QColor object.
     *
     */
    void setColor(const QColor color);

    /** \brief Returns the color of the brush.
     *
     */
    QColor color() const;

    /** \brief Sets the border color of the brush cursor.
     * \param[in] color QColor object.
     *
     */
    void setBorderColor(const QColor borderColor);

    /** \brief Returns the border color of the bursh cursor.
     *
     */
    QColor borderColor() const;

    /** \brief Sets the image of the cursor.
     * \param[in] image QImage object.
     *
     */
    void setImage(const QImage &image);

    /** \brief Resets the image of the cursor.
     *
     */
    void clearImage();

    /** \brief Sets the radius of the brush cursor.
     * \param[in] value radius value > 0.
     *
     */
    void setRadius(const int value);

    /** \brief Returs the radius size of the brush cursor.
     *
     */
    int radius() const;

    /** \brief Sets the origin point of the image under cursor to be used by the brush.
     * \param[in] origin origin point.
     *
     */
    void setOrigin(const NmVector3 &origin);

    /** \brief Returns the origin point of the image under cursor.
     *
     */
    NmVector3 origin() const;

    /** \brief Sets the spacing of the image under cursor to be used by the brush.
     * \param[in] spacing spacing vector.
     *
     */
    void setSpacing(const NmVector3 &spacing);

    /** \brief Returns the spacing of the image under cursor.
     *
     */
    NmVector3 spacing() const;

  signals:
    void strokeStarted(RenderView *view);
    void strokeUpdated(Brush::Stroke stroke);
    void strokeFinished(Brush::Stroke, RenderView *view);
    void radiusChanged(int value);

  private slots:
    /** \brief Called when an stroke starts.
     * \param[in] track stroke points.
     * \param[in] view view under cursor.
     *
     */
    void onTrackStarted(Track track, RenderView *view);

    /** \brief Called when the stroke is updated.
     * \param[in] track stroke points.
     * \param[in] view view under cursor.
     *
     */
    void onTrackUpdated(Track track);

    /** \brief Called when the stroke ends.
     * \param[in] track stroke points.
     * \param[in] view view under cursor.
     *
     */
    void onTrackStopped(Track track, RenderView *view);

  private:
    /** \brief Interpolates the track points to meet the point distance of the radius/2.
     * \param[in] track points in the stroke.
     *
     */
    Stroke createStroke(Track track);

    /** \brief Returns the StrokePoint created for track point.
     * \param[in] point track point.
     *
     */
    virtual StrokePoint createStrokePoint(NmVector3 point) = 0;

    /** \brief Configures the brush cursor.
     * \param[in] view view under the cursor.
     *
     */
    virtual void configureBrush(RenderView *view) = 0;

    /** \brief Updates the brush cursor.
     *
     */
    void updateCursor();

  private:
    QColor    m_color;       /** cursor color.               */
    QColor    m_borderColor; /** cursor border color.        */
    int       m_radius;      /** cursor radius.              */
    QVariant  m_image;       /** cursor image.               */
    NmVector3 m_spacing;     /** image under cursor spacing. */
    NmVector3 m_origin;      /** image under cursor origin.  */
    Stroke    m_stroke;      /** stroke points.              */
  };

  using BrushSPtr = std::shared_ptr<Brush>;
}

#endif // ESPINA_BRUSH_H
