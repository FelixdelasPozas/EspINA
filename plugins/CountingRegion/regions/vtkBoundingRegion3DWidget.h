/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2011  Jorge Peña Pastor <jpena@cesvima.upm.es>

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

#ifndef VTKBOUNDINGREGION3DWIDGET_H
#define VTKBOUNDINGREGION3DWIDGET_H

#include "vtkBoundingRegionWidget.h"

class vtkPolyData;
class vtkPolyDataAlgorithm;

class VTK_WIDGETS_EXPORT vtkBoundingRegion3DWidget
: public vtkBoundingRegionWidget
{
public:
  // Description:
  // Instantiate the object.
  static vtkBoundingRegion3DWidget *New();

  // Description:
  // Standard class methods for type information and printing.
  vtkTypeMacro(vtkBoundingRegion3DWidget, vtkBoundingRegionWidget);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual void SetBoundingRegion(vtkSmartPointer< vtkPolyData > region, Nm inclusionOffset[3], Nm exclusionOffset[3]);

  // Description:
  // Create the default widget representation if one is not set. By default,
  // this is an instance of the vtkBoundingRegion3DRepresentation class.
  void CreateDefaultRepresentation();

protected:
  vtkBoundingRegion3DWidget();
  ~vtkBoundingRegion3DWidget();

//BTX - manage the state of the widget
  int WidgetState;
  enum _WidgetState {Start=0,Active};
//ETX

  // These methods handle events
  static void SelectAction(vtkAbstractWidget*);
  static void EndSelectAction(vtkAbstractWidget*);

  // helper methods for cursoe management
  virtual void SetCursor(int state);

  vtkPolyData *Volume;

private:
  vtkBoundingRegion3DWidget(const vtkBoundingRegion3DWidget&);  //Not implemented
  void operator=(const vtkBoundingRegion3DWidget&);  //Not implemented
};

#endif // VTKBOUNDINGREGION3DWIDGET_H
