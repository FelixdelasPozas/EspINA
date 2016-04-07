/*

    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>

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

#ifndef ESPINA_VIEW_2D_STATE_H
#define ESPINA_VIEW_2D_STATE_H

#include "GUI/EspinaGUI_Export.h"

// ESPINA
#include "GUI/View/View2D.h"

class vtkCamera;
class vtkPolyData;
class vtkProp3D;

namespace ESPINA
{
  /** \class PlanarBehaviour
   * \brief Defines the common methods for planar views.
   *
   */
  class EspinaGUI_EXPORT View2D::PlanarBehaviour
  {
    public:
      /** \brief PlanarBehaviour class constructor.
       *
       */
      virtual ~PlanarBehaviour()
      {}

      /** \brief Updates the camera of the view.
       *
       */
      virtual void updateCamera(vtkCamera       *camera,
                                const NmVector3 &center) const = 0;
  };

  /** \class AxialBehaviour
   * \brief Implements the common methods for axial planar views.
   *
   */
  class EspinaGUI_EXPORT View2D::AxialBehaviour
  : public View2D::PlanarBehaviour
  {
    public:
      /** \brief AxialBehaviour class constructor.
       *
       */
      explicit AxialBehaviour()
      {}

      virtual void updateCamera(vtkCamera       *camera,
                                const NmVector3 &center) const;
  };

  /** \class SagittalBehaviour
   * \brief Implements the common methods for sagittal planar views.
   *
   */
  class EspinaGUI_EXPORT View2D::SagittalBehaviour
  : public View2D::PlanarBehaviour
  {
    public:
      /** \brief SagittalState class constructor.
       *
       */
      explicit SagittalBehaviour()
      {}

      virtual void updateCamera(vtkCamera       *camera,
                                const NmVector3 &center) const;
  };

  /** \class CoronalBehaviour
   * \brief Implements the common methods for coronal planar views.
   *
   */
  class EspinaGUI_EXPORT View2D::CoronalBehaviour
  : public View2D::PlanarBehaviour
  {
    public:
      /** \brief CoronalState class constructor.
       *
       */
      explicit CoronalBehaviour()
      {}

      virtual void updateCamera(vtkCamera       *camera,
                                const NmVector3 &center) const;
  };
} // namespace ESPINA

#endif // ESPINA_VIEW_2D_STATE_H
