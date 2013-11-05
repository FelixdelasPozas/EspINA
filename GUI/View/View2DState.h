/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  Jorge PeÃ±a Pastor <email>

    This program is free software: you can redistribute it and/or modify
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

#include "GUI/View/View2D.h"

class vtkCamera;
class vtkPolyData;
class vtkProp3D;

namespace EspINA
{
  class View2D::State
  {
  public:
    virtual ~State() {}

    virtual void setCrossHairs(vtkPolyData*     hline,
                               vtkPolyData*     vline,
                               const NmVector3& center,
                               const Bounds&    bounds,
                               const NmVector3& slicingStep) = 0;

    virtual void updateActor(vtkProp3D *actor) = 0;

    virtual void updateCamera(vtkCamera *camera,
                              const NmVector3& center) = 0;
  };

  class View2D::AxialState
  : public View2D::State
  {
  public:
    explicit AxialState(){}

    virtual void setCrossHairs(vtkPolyData*     hline,
                               vtkPolyData*     vline,
                               const NmVector3& center,
                               const Bounds&    bounds,
                               const NmVector3& slicingStep);
    virtual void updateActor(vtkProp3D *actor) {};
    virtual void updateCamera(vtkCamera *camera,
                              const NmVector3& center);
  };

  class View2D::SagittalState
  : public View2D::State
  {
  public:
    explicit SagittalState(){}

    virtual void setCrossHairs(vtkPolyData*     hline,
                               vtkPolyData*     vline,
                               const NmVector3& center,
                               const Bounds&    bounds,
                               const NmVector3& slicingStep);
    virtual void updateActor(vtkProp3D *actor);
    virtual void updateCamera(vtkCamera *camera,
                              const NmVector3& center);
  };

  class View2D::CoronalState
  : public View2D::State
  {
  public:
    explicit CoronalState(){}

    virtual void setCrossHairs(vtkPolyData*     hline,
                               vtkPolyData*     vline,
                               const NmVector3& center,
                               const Bounds&    bounds,
                               const NmVector3& slicingStep);
    virtual void updateActor(vtkProp3D *actor);
    virtual void updateCamera(vtkCamera *camera,
                              const NmVector3& center);
  };
} // namespace EspINA

#endif // ESPINA_VIEW_2D_STATE_H
