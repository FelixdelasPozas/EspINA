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


#ifndef VTKBOUNDINGREGIONCORONALSLICEREPRESENTATION_H
#define VTKBOUNDINGREGIONCORONALSLICEREPRESENTATION_H

#include <regions/vtkBoundingRegionSliceRepresentation.h>


class vtkBoundingRegionCoronalSliceRepresentation
: public vtkBoundingRegionSliceRepresentation
{
public:
  static vtkBoundingRegionCoronalSliceRepresentation *New();

  vtkTypeMacro(vtkBoundingRegionCoronalSliceRepresentation,
               vtkBoundingRegionSliceRepresentation);

  virtual void SetSlice(Nm pos);

protected:
  static const int hCoord = 0;
  static const int vCoord = 2;

  virtual void CreateRegion();

  virtual Nm realLeftEdge  (int slice=0) { return Region->GetPoint(slice*4+1)[hCoord];}
  virtual Nm realTopEdge   (int slice=0) { return Region->GetPoint(0)[vCoord];}
  virtual Nm realRightEdge (int slice=0) { return Region->GetPoint(slice*4+3)[hCoord];}
  virtual Nm realBottomEdge(int slice=0) { return Region->GetPoint(NumPoints-4)[vCoord];}

  virtual Nm leftEdge  (int slice=0) {return realLeftEdge  (slice) + InclusionOffset[hCoord];}
  virtual Nm topEdge   (int slice=0) {return realTopEdge   (slice) + InclusionOffset[vCoord];}
  virtual Nm rightEdge (int slice=0) {return realRightEdge (slice) - ExclusionOffset[hCoord];}
  virtual Nm bottomEdge(int slice=0) {return realBottomEdge(slice) - ExclusionOffset[vCoord];}

  virtual void MoveLeftEdge  (double* p1, double* p2);
  virtual void MoveRightEdge (double* p1, double* p2);
  virtual void MoveTopEdge   (double* p1, double* p2);
  virtual void MoveBottomEdge(double* p1, double* p2);

protected:
  explicit vtkBoundingRegionCoronalSliceRepresentation(){}

private:
  vtkBoundingRegionCoronalSliceRepresentation(const vtkBoundingRegionCoronalSliceRepresentation&);  //Not implemented
  void operator=(const vtkBoundingRegionCoronalSliceRepresentation&);  //Not implemented
};

#endif // VTKBOUNDINGREGIONCORONALSLICEREPRESENTATION_H