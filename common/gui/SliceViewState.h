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


#ifndef SLICEVIEW_STATE_H
#define SLICEVIEW_STATE_H

#include "gui/SliceView.h"

class vtkCamera;
class vtkMatrix4x4;
class vtkPolyData;
class vtkProp3D;

class SliceView::State
{
public:
  virtual ~State() {}

  virtual void setCrossHairs(vtkPolyData *hline,
			    vtkPolyData *vline,
			    double center[3],
			    double bounds[6] ) = 0;

  virtual void setSlicePosition(vtkMatrix4x4 *matrix,
			       double value) = 0;
  virtual void updateActor(vtkProp3D *actor) = 0;
  virtual void updateCamera(vtkCamera *camera,
			   double center[3]) = 0;

  virtual void updateSlicingMatrix(vtkMatrix4x4 *matrix) = 0;
};

class SliceView::AxialState
: public SliceView::State
{
public:
  explicit AxialState(){}

  virtual void setCrossHairs(vtkPolyData* hline,
			     vtkPolyData* vline,
			     double center[3],
			     double bounds[6]);
  virtual void setSlicePosition(vtkMatrix4x4 *matrix,
				double value);
  virtual void updateActor(vtkProp3D *actor){}
  virtual void updateCamera(vtkCamera *camera,
                            double center[3]);
  virtual void updateSlicingMatrix(vtkMatrix4x4 *matrix);
};

class SliceView::SagittalState
: public SliceView::State
{
public:
  explicit SagittalState(){}

  virtual void setCrossHairs(vtkPolyData* hline,
			     vtkPolyData* vline,
			     double center[3],
			     double bounds[6]);
  virtual void setSlicePosition(vtkMatrix4x4 *matrix,
				double value);
  virtual void updateActor(vtkProp3D *actor);
  virtual void updateCamera(vtkCamera *camera,
                            double center[3]);
  virtual void updateSlicingMatrix(vtkMatrix4x4 *matrix);
};

class SliceView::CoronalState
: public SliceView::State
{
public:
  explicit CoronalState(){}

  virtual void setCrossHairs(vtkPolyData* hline,
			     vtkPolyData* vline,
			     double center[3],
			     double bounds[6]);
  virtual void setSlicePosition(vtkMatrix4x4 *matrix,
				double value);
  virtual void updateActor(vtkProp3D *actor);
  virtual void updateCamera(vtkCamera *camera,
                            double center[3]);
  virtual void updateSlicingMatrix(vtkMatrix4x4 *matrix);
};
#endif // SLICEVIEW_STATE_H