/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  Jorge Peña Pastor <jpena@cesvima.upm.es>

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

#ifndef VTKRECTANGULARWIDGET_H
#define VTKRECTANGULARWIDGET_H

#include "vtkAbstractWidget.h"

#include <common/views/vtkPVSliceView.h>

class vtkPolyDataAlgorithm;
class vtkRectangularRepresentation;
class vtkHandleWidget;

class VTK_WIDGETS_EXPORT vtkRectangularWidget : public vtkAbstractWidget
{
public:
  // Description:
  // Instantiate the object.
  static vtkRectangularWidget *New();

  // Description:
  // Standard class methods for type information and printing.
  vtkTypeMacro(vtkRectangularWidget,vtkAbstractWidget);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Specify an instance of vtkWidgetRepresentation used to represent this
  // widget in the scene. Note that the representation is a subclass of vtkProp
  // so it can be added to the renderer independent of the widget.
  void SetRepresentation(vtkRectangularRepresentation *r)
    {this->Superclass::SetWidgetRepresentation(reinterpret_cast<vtkWidgetRepresentation*>(r));}

  // Description:
  // Control the behavior of the widget (i.e., how it processes
  // events). Translation, rotation, and scaling can all be enabled and
  // disabled.
  vtkSetMacro(TranslationEnabled,int);
  vtkGetMacro(TranslationEnabled,int);
  vtkBooleanMacro(TranslationEnabled,int);
  vtkSetMacro(ScalingEnabled,int);
  vtkGetMacro(ScalingEnabled,int);
  vtkBooleanMacro(ScalingEnabled,int);

  virtual void SetPlane(vtkPVSliceView::VIEW_PLANE plane);
//   virtual void SetSlice(int slice, double spacing[3]);
//   virtual void SetRegion(vtkPolyDataAlgorithm *region);

  // Description:
  // Create the default widget representation if one is not set. By default,
  // this is an instance of the vtkRectangularRepresentation class.
  void CreateDefaultRepresentation();

protected:
  vtkRectangularWidget();
  ~vtkRectangularWidget();

//BTX - manage the state of the widget
  int WidgetState;
  enum _WidgetState {Start=0,Active};
//ETX

  // These methods handle events
  static void SelectAction(vtkAbstractWidget*);
  static void EndSelectAction(vtkAbstractWidget*);
  static void TranslateAction(vtkAbstractWidget*);
  static void ScaleAction(vtkAbstractWidget*);
  static void MoveAction(vtkAbstractWidget*);

  // helper methods for cursor management
  virtual void SetCursor(int state);

  // Control whether scaling and translation are supported
  int TranslationEnabled;
  int ScalingEnabled;

  vtkPVSliceView::VIEW_PLANE Plane;

//   vtkPolyDataAlgorithm *Region;
private:
  vtkRectangularWidget(const vtkRectangularWidget&);  //Not implemented
  void operator=(const vtkRectangularWidget&);  //Not implemented
};

#endif // VTKRECTANGULARWIDGET_H