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

// ESPINA
#include "GUI/View/View2D.h"

class vtkCamera;
class vtkPolyData;
class vtkProp3D;

namespace ESPINA
{
  class View2D::State
  {
  public:
  	/* \brief State class constructor.
  	 *
  	 */
    virtual ~State()
    {}

    /* \brief Sets the crosshairs of the view.
     * \param[out] hline, smart pointer of the vtkPolyData of the horizontal crosshair line.
     * \param[out] vline, smart pointer of the vtkPolyData of the vertical crosshair line.
     * \param[in] center, crosshair point.
     * \param[in] bounds, bounds of the view.
     * \param[in] slicingStep, spacing of the view's plane.
     *
     */
    virtual void setCrossHairs(vtkSmartPointer<vtkPolyData> &hline,
                               vtkSmartPointer<vtkPolyData> &vline,
                               const NmVector3              &center,
                               const Bounds                 &bounds,
                               const NmVector3              &slicingStep) = 0;

    /* \brief Updates the camera of the view.
     *
     */
    virtual void updateCamera(vtkCamera       *camera,
                              const NmVector3 &center) = 0;
  };

  class View2D::AxialState
  : public View2D::State
  {
  public:
  	/* \brief AxialState class constructor.
  	 *
  	 */
    explicit AxialState()
    {}

    /* \brief Implements State::setCrosshairs().
     *
     */
    virtual void setCrossHairs(vtkSmartPointer<vtkPolyData> &hline,
                               vtkSmartPointer<vtkPolyData> &vline,
                               const NmVector3              &center,
                               const Bounds                 &bounds,
                               const NmVector3              &slicingStep);

    /* \brief Implements State::updateCamera().
     *
     */
    virtual void updateCamera(vtkCamera       *camera,
                              const NmVector3 &center);
  };

  class View2D::SagittalState
  : public View2D::State
  {
  public:
  	/* \brief SagittalState class constructor.
  	 *
  	 */
    explicit SagittalState()
    {}

    /* \brief Implements State::setCrosshairs().
     *
     */
    virtual void setCrossHairs(vtkSmartPointer<vtkPolyData> &hline,
                               vtkSmartPointer<vtkPolyData> &vline,
                               const NmVector3              &center,
                               const Bounds                 &bounds,
                               const NmVector3              &slicingStep);

    /* \brief Implements State::updateCamera().
     *
     */
    virtual void updateCamera(vtkCamera       *camera,
                              const NmVector3 &center);
  };

  class View2D::CoronalState
  : public View2D::State
  {
  public:
  	/* \brief CoronalState class constructor.
  	 *
  	 */
    explicit CoronalState()
    {}

    /* \brief Implements State::setCrosshairs().
     *
     */
    virtual void setCrossHairs(vtkSmartPointer<vtkPolyData> &hline,
                               vtkSmartPointer<vtkPolyData> &vline,
                               const NmVector3              &center,
                               const Bounds                 &bounds,
                               const NmVector3              &slicingStep);

    /* \brief Implements State::updateCamera().
     *
     */
    virtual void updateCamera(vtkCamera       *camera,
                              const NmVector3 &center);
  };
} // namespace ESPINA

#endif // ESPINA_VIEW_2D_STATE_H
