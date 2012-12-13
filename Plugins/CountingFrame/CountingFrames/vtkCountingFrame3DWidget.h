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

#ifndef VTKBOUNDINGFRAME3DWIDGET_H
#define VTKBOUNDINGFRAME3DWIDGET_H

#include "vtkCountingFrameWidget.h"

class vtkPolyData;
class vtkPolyDataAlgorithm;

class VTK_WIDGETS_EXPORT vtkCountingFrame3DWidget
: public vtkCountingFrameWidget
{
public:
  // Description:
  // Instantiate the object.
  static vtkCountingFrame3DWidget *New();

  // Description:
  // Standard class methods for type information and printing.
  vtkTypeMacro(vtkCountingFrame3DWidget, vtkCountingFrameWidget);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual void SetCountingFrame(vtkSmartPointer< vtkPolyData > region, Nm inclusionOffset[3], Nm exclusionOffset[3]);

  // Description:
  // Create the default widget representation if one is not set. By default,
  // this is an instance of the vtkCountingFrame3DRepresentation class.
  void CreateDefaultRepresentation();

protected:
  vtkCountingFrame3DWidget();
  ~vtkCountingFrame3DWidget();

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
  vtkCountingFrame3DWidget(const vtkCountingFrame3DWidget&);  //Not implemented
  void operator=(const vtkCountingFrame3DWidget&);  //Not implemented
};

#endif // VTKBOUNDINGFRAME3DWIDGET_H
