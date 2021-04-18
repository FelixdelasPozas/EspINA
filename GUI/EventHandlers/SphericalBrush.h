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

#ifndef ESPINA_SPHERICAL_BRUSH_H_
#define ESPINA_SPHERICAL_BRUSH_H_

#include <GUI/EspinaGUI_Export.h>

// ESPINA
#include <GUI/EventHandlers/Brush.h>

namespace ESPINA
{
  /** \class SphericalBrush
   * \brief Implements a spherical 3D brush.
   *
   */
  class EspinaGUI_EXPORT SphericalBrush
  : public Brush
  {
    private:
      virtual StrokePoint createStrokePoint(NmVector3 point);

      virtual void configureBrush(RenderView* view);

    private:
      Nm        m_strokeRadius; /** radius of the brush.     */
      Plane     m_plane;        /** plane of the cursor.     */
      NmVector3 m_resolution;   /** resolution of the image. */
  };

} // namespace ESPINA

#endif // ESPINA_SPHERICAL_BRUSH_H_
