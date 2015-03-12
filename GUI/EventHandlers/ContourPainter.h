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

// ESPINA
#include <GUI/EventHandlers/MaskPainter.h>


namespace ESPINA
{
  class ContourWidget;
  
  class ContourPainter
  : public MaskPainter
  {
    public:
      ContourPainter();

      virtual bool filterEvent(QEvent *e, RenderView *view = nullptr) override;

      /** \brief Sets the contour widget of this painter.
       *
       */
      void setContourWidget(ContourWidget *widget);

      /** \brief Sets the minimum distance between contour points when doing a stroke.
       * \param[in] distance minimum distance between points in a stroke.
       *
       */
      void setMinimumPointDistance(Nm distance);

      /** \brief Returns the minimum distance between points in a stroke.
       *
       */
      Nm minimumPointDistance() const
      { return m_minDistance; }

    private:
      virtual void updateCursor(DrawingMode mode);

      virtual void onMaskPropertiesChanged(const NmVector3 &spacing, const NmVector3 &origin=NmVector3());

      /** \brief Helper method to update the contour border and mode properties.
       *
       */
      void updateContourWidget();

    private:
      Nm             m_minDistance;
      bool           m_tracking;
      NmVector3      m_maskSpacing;
      ContourWidget *m_widget;
  };

} // namespace ESPINA

#endif // ESPINA_CONTOUR_PAINTER_H_
