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

#ifndef ESPINA_CIRCULAR_BRUSH_H
#define ESPINA_CIRCULAR_BRUSH_H

#include <GUI/EspinaGUI_Export.h>

// ESPINA
#include <GUI/EventHandlers/Brush.h>

namespace ESPINA
{
  /** \class CircularBrush
   * \brief Circular 2D brush.
   *
   */
  class EspinaGUI_EXPORT CircularBrush
  : public Brush
  {
  private:
    virtual StrokePoint createStrokePoint(NmVector3 point);

    virtual void configureBrush(RenderView* view);

  private:
    Nm        m_strokeRadius; /** radius of the stroke points.   */
    Plane     m_plane;        /** orthogonal plane of the brush. */
    NmVector3 m_resolution;   /** resolution of the plane.       */
  };
}

#endif // ESPINA_CIRCULAR_BRUSH_H
