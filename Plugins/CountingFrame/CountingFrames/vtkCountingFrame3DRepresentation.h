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

#ifndef VTKBOUNDINGFRAME3DRESENTATION_H
#define VTKBOUNDINGFRAME3DRESENTATION_H

// ESPINA
#include "CountingFramePlugin_Export.h"
#include "vtkWidgetRepresentation.h"
#include <Core/Types.h>

// VTK
#include <vtkSmartPointer.h>

class vtkActor;
class vtkLookupTable;
class vtkPolyDataMapper;
class vtkPolyData;
class vtkProperty;
class vtkViewPort;
class vtkPoints;

class CountingFramePlugin_EXPORT vtkCountingFrame3DRepresentation
: public vtkWidgetRepresentation
{
  //ETX
public:
  // Description:
  // Instantiate the class.
  static vtkCountingFrame3DRepresentation *New();

  // Description:
  // Standard methods for the class.
  vtkTypeMacro(vtkCountingFrame3DRepresentation, vtkWidgetRepresentation);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual void SetCountingFrame(vtkSmartPointer<vtkPolyData> region);

  // Description:
  // These are methods that satisfy vtkWidgetRepresentation's API.
  virtual void BuildRepresentation();
  virtual double *GetBounds();

  // Description:
  // Methods supporting, and required by, the rendering process.
  virtual void ReleaseGraphicsResources(vtkWindow*) override;
  virtual int  RenderOpaqueGeometry(vtkViewport*) override;
  virtual int  RenderTranslucentPolygonalGeometry(vtkViewport*) override;
  virtual int  HasTranslucentPolygonalGeometry() override;
  virtual void SetVisibility(int visible) override;
  virtual void VisibilityOn() override;
  virtual void VisibilityOff() override;

protected:
  vtkCountingFrame3DRepresentation();
  ~vtkCountingFrame3DRepresentation();

  // 3D Volume
  vtkSmartPointer<vtkPolyData>       CountingFrame;
  vtkSmartPointer<vtkPolyDataMapper> VolumeMapper;
  vtkSmartPointer<vtkLookupTable>    InclusionLUT;
  vtkSmartPointer<vtkActor>          VolumeActor;
  vtkSmartPointer<vtkPoints>         VolumePoints;
  vtkSmartPointer<vtkProperty>       Property;

  int Visible;

private:
  vtkCountingFrame3DRepresentation(const vtkCountingFrame3DRepresentation&);  //Not implemented
  void operator=(const vtkCountingFrame3DRepresentation&);  //Not implemented
};

#endif
