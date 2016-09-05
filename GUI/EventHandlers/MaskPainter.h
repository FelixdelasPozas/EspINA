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

#ifndef ESPINA_MASK_PAINTER_H
#define ESPINA_MASK_PAINTER_H

#include <GUI/EspinaGUI_Export.h>

// ESPINA
#include <GUI/EventHandlers/PointTracker.h>
#include <Core/Utils/BinaryMask.hxx>
#include "StrokePainter.h"

namespace ESPINA
{
  class EspinaGUI_EXPORT MaskPainter
  : public EventHandler
  {
    Q_OBJECT
  public:

  public:
    virtual bool filterEvent(QEvent *e, RenderView *view = nullptr) override;

    /** \brief Enables/Disables the ability to erase of the painter.
     * \param[in] value true to enable erasing and false otherwise.
     *
     */
    void setCanErase(bool value);

    /** \brief Returns true if the painter can erase.
     *
     */
    bool canErase()
    { return m_canErase; };

    /** \brief Sets the drawing mode to the specified one.
     * \param[in] mode drawing mode.
     *
     */
    void setDrawingMode(const DrawingMode mode);

    /** \brief Returns the drawing mode of the painter.
     *
     */
    DrawingMode drawingMode() const
    { return m_mode; }

    /** \brief Sets the color of the painter.
     * \param[in] color color of the painter.
     *
     */
    virtual void setColor(const QColor &color);

    /** \brief Returns the color of the painter.
     *
     */
    QColor color() const
    { return m_color; };

    void setMaskProperties(const NmVector3 &spacing, const NmVector3 &origin=NmVector3());

    /** \brief Implements PointTracker::setInterpolation().
     *
     */
    void setInterpolation(bool active)
    { m_tracker->setInterpolation(active); }

    /** \brief Implements PointTracker::interpolationEnabled().
     *
     */
    bool interpolationEnabled() const
    { return m_tracker->interpolationEnabled(); }

    /** \brief Implements PointTracker::setMaximumPointDistance().
     *
     */
    void setMaximumPointDistance(Nm distance)
    { m_tracker->setMaximumPointDistance(distance); }

    /** \brief Implements PointTracker::interpolationDistance().
     *
     */
    Nm interpolationDistance() const
    { return m_tracker->interpolationDistance(); }

    /** \brief Implements PointTracker::isTracking().
     *
     */
    bool isTracking() const
    { return m_tracker->isTracking(); }

  protected:
    explicit MaskPainter(PointTrackerSPtr handler);

    /** \brief This methods takes into account the shift modifier
     *
     */
    DrawingMode currentMode() const;

  signals:
    void drawingModeChanged(DrawingMode mode) const;

    void startPainting() const;

    void stopPainting(BinaryMaskSPtr<unsigned char> mask) const;

    void clear() const;

  private:
    /** \brief Helper method to update brush when drawing mode changes.
     *
     */
    void updateDrawingMode();

    /** \brief Returns true when the shift key is down in the keyboard.
     *
     */
    inline bool ShiftKeyIsDown() const;

    /** \brief Helper method to update the brush cursor when the drawing mode changes.
     * \param[in] mode drawing mode.
     *
     */
    virtual void updateCursor(DrawingMode mode) = 0;

    /** \brief Helper method to update mask properties.
     * \param[in] spacing spacing of the mask.
     * \param[in] origin origin point of the mask.
     *
     */
    virtual void onMaskPropertiesChanged(const NmVector3 &spacing, const NmVector3 &origin=NmVector3()) = 0;

  protected:
    NmVector3        m_origin;   /** origin of the mask.                     */
    NmVector3        m_spacing;  /** spacing of the mask.                    */
    QColor           m_color;    /** painting color.                         */

  private:
    PointTrackerSPtr m_tracker;  /** point tracker.                          */
    bool             m_canErase; /** true if the brush can enter erase mode. */
    DrawingMode      m_mode;     /** current drawing mode.                   */
  };

  using MaskPainterSPtr = std::shared_ptr<MaskPainter>;
}

#endif // ESPINA_MASKPAINTER_H
