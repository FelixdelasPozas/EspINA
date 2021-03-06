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

#ifndef ESPINA_CONTOUR_PAINTER_H_
#define ESPINA_CONTOUR_PAINTER_H_

#include <GUI/EspinaGUI_Export.h>

// ESPINA
#include <GUI/EventHandlers/MaskPainter.h>

namespace ESPINA
{
  /** \class ContourPainter
   * \brief Event handler for contour tool.
   *
   */
  class EspinaGUI_EXPORT ContourPainter
  : public MaskPainter
  {
      Q_OBJECT
    public:
      /** \brief ContourPainter class constructor.
       *
       */
      ContourPainter();

      virtual bool filterEvent(QEvent *e, RenderView *view = nullptr) override;

      /** \brief Sets the minimum distance between contour points when doing a stroke.
       * \param[in] distance minimum distance between points in a stroke.
       *
       */
      void setMinimumPointDistance(Nm distance);

      /** \brief Helper method to update the widgets parameters.
       *
       */
      void updateWidgetsValues() const;

      /** \brief Forces rasterization of the actual contours (if any exists).
       *
       */
      void rasterizeContours();

    signals:
      void configure(Nm distance, QColor color, NmVector3 spacing) const;

      void rasterize() const;

    private:
      /** \brief Updates the cursor when the drawing mode changes.
       * \param[in] mode drawing mode.
       *
       */
      virtual void updateCursor(DrawingMode mode) override;

      /** \brief Updates the spacing and origin of the generated mask.
       *
       */
      virtual void onMaskPropertiesChanged(const NmVector3 &spacing, const NmVector3 &origin=NmVector3()) override;

    private:
      Nm             m_minDistance; /** point minimum distance.                      */
      bool           m_tracking;    /** true if tracking points and false otherwise. */
      NmVector3      m_maskSpacing; /** spacing of the mask.                         */
  };

  using ContourPainterSPtr = std::shared_ptr<ContourPainter>;

} // namespace ESPINA

#endif // ESPINA_CONTOUR_PAINTER_H_
