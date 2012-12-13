/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  <copyright holder> <email>

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


#ifndef VTKBOUNDINGFRAMESAGITTALSLICEREPRESENTATION_H
#define VTKBOUNDINGFRAMESAGITTALSLICEREPRESENTATION_H

#include "CountingFrames/vtkCountingFrameSliceRepresentation.h"


class vtkCountingFrameSagittalSliceRepresentation
: public vtkCountingFrameSliceRepresentation
{
public:
  static vtkCountingFrameSagittalSliceRepresentation *New();

  vtkTypeMacro(vtkCountingFrameSagittalSliceRepresentation,
               vtkCountingFrameSliceRepresentation);

  virtual void SetSlice(Nm pos);

protected:
  virtual void CreateRegion();

  virtual Nm realLeftEdge  (int slice=0) { return Region->GetPoint(0)[2];}
  virtual Nm realTopEdge   (int slice=0) { return Region->GetPoint(slice*4+1)[1];}
  virtual Nm realRightEdge (int slice=0) { return Region->GetPoint(NumPoints-1)[2];}
  virtual Nm realBottomEdge(int slice=0) { return Region->GetPoint(slice*4+0)[1];}

  virtual Nm leftEdge  (int slice=0) {return realLeftEdge  (slice) + InclusionOffset[2];}
  virtual Nm topEdge   (int slice=0) {return realTopEdge   (slice) + InclusionOffset[1];}
  virtual Nm rightEdge (int slice=0) {return realRightEdge (slice) - ExclusionOffset[2];}
  virtual Nm bottomEdge(int slice=0) {return realBottomEdge(slice) - ExclusionOffset[1];}

  virtual void MoveLeftEdge  (double* p1, double* p2);
  virtual void MoveRightEdge (double* p1, double* p2);
  virtual void MoveTopEdge   (double* p1, double* p2);
  virtual void MoveBottomEdge(double* p1, double* p2);

protected:
  explicit vtkCountingFrameSagittalSliceRepresentation(){}

private:
  vtkCountingFrameSagittalSliceRepresentation(const vtkCountingFrameSagittalSliceRepresentation&);  //Not implemented
  void operator=(const vtkCountingFrameSagittalSliceRepresentation&);  //Not implemented
};

#endif // VTKBOUNDINGFRAMESAGITTALSLICEREPRESENTATION_H