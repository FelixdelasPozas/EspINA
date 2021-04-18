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


#ifndef VTK_COUNTING_FRAME_REPRESENTATION_YZ_H
#define VTK_COUNTING_FRAME_REPRESENTATION_YZ_H

#include "CountingFramePlugin_Export.h"

#include "CountingFrames/vtkCountingFrameSliceRepresentation.h"

class CountingFramePlugin_EXPORT vtkCountingFrameRepresentationYZ
: public vtkCountingFrameSliceRepresentation
{
public:
  static vtkCountingFrameRepresentationYZ *New();

  vtkTypeMacro(vtkCountingFrameRepresentationYZ,
               vtkCountingFrameSliceRepresentation);

  virtual void SetSlice(ESPINA::Nm pos);

protected:
  virtual void CreateRegion();

  virtual ESPINA::Nm realLeftEdge  (int slice=0)
  {
    double point[3];
    Region->GetPoint(0, point);
    return point[2];
  }

  virtual ESPINA::Nm realTopEdge   (int slice=0)
  {
    double point[3];
    Region->GetPoint(slice*4+1, point);
    return point[1];
  }

  virtual ESPINA::Nm realRightEdge (int slice=0)
  {
    double point[3];
    Region->GetPoint(NumPoints-1, point);
    return point[2];
  }

  virtual ESPINA::Nm realBottomEdge(int slice=0)
  {
    double point[3];
    Region->GetPoint(slice*4+0, point);
    return point[1];
  }

  virtual ESPINA::Nm leftEdge  (int slice=0) {return realLeftEdge  (slice) + InclusionOffset[2];}
  virtual ESPINA::Nm topEdge   (int slice=0) {return realTopEdge   (slice) + InclusionOffset[1];}
  virtual ESPINA::Nm rightEdge (int slice=0) {return realRightEdge (slice) - ExclusionOffset[2];}
  virtual ESPINA::Nm bottomEdge(int slice=0) {return realBottomEdge(slice) - ExclusionOffset[1];}

  virtual void MoveLeftEdge  (double* p1, double* p2);
  virtual void MoveRightEdge (double* p1, double* p2);
  virtual void MoveTopEdge   (double* p1, double* p2);
  virtual void MoveBottomEdge(double* p1, double* p2);

protected:
  explicit vtkCountingFrameRepresentationYZ(){}

private:
  vtkCountingFrameRepresentationYZ(const vtkCountingFrameRepresentationYZ&) = delete;
  void operator=(const vtkCountingFrameRepresentationYZ&) = delete;
};

#endif // VTK_COUNTING_FRAME_REPRESENTATION_YZ_H
