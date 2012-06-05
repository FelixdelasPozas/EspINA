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

#ifndef VTKRECTANGULARBOUNDINGREGIONWIDGET_H
#define VTKRECTANGULARBOUNDINGREGIONWIDGET_H

#include "vtkAbstractWidget.h"
#include <common/views/vtkPVSliceView.h>

class vtkPolyDataAlgorithm;
class vtkRectangularBoundingRegionRepresentation;

class VTK_WIDGETS_EXPORT vtkRectangularBoundingRegionWidget
: public vtkAbstractWidget
{
public:
  // Description:
  // Instantiate the object.
  static vtkRectangularBoundingRegionWidget *New();

  // Description:
  // Standard class methods for type information and printing.
  vtkTypeMacro(vtkRectangularBoundingRegionWidget,vtkAbstractWidget);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Specify an instance of vtkWidgetRepresentation used to represent this
  // widget in the scene. Note that the representation is a subclass of vtkProp
  // so it can be added to the renderer independent of the widget.
  void SetRepresentation(vtkRectangularBoundingRegionRepresentation *r)
    {this->Superclass::SetWidgetRepresentation(reinterpret_cast<vtkWidgetRepresentation*>(r));}

  vtkSetVector3Macro(InclusionOffset, double);
  vtkGetVector3Macro(InclusionOffset, double);
  vtkSetVector3Macro(ExclusionOffset, double);
  vtkGetVector3Macro(ExclusionOffset, double);

  virtual void SetPlane(vtkPVSliceView::VIEW_PLANE plane);
  virtual void SetSlice(double pos/*nm*/);
  virtual void SetRegion(vtkPolyDataAlgorithm *region);

  // Description:
  // Create the default widget representation if one is not set. By default,
  // this is an instance of the vtkRectangularBoundingRegionRepresentation class.
  void CreateDefaultRepresentation();

protected:
  vtkRectangularBoundingRegionWidget();
  ~vtkRectangularBoundingRegionWidget();

//BTX - manage the state of the widget
  int WidgetState;
  enum _WidgetState {Start=0,Active};
//ETX

  // These methods handle events
  static void SelectAction(vtkAbstractWidget*);
  static void EndSelectAction(vtkAbstractWidget*);
  static void TranslateAction(vtkAbstractWidget*);
  static void MoveAction(vtkAbstractWidget*);
  
  // helper methods for cursoe management
  virtual void SetCursor(int state);

  double InclusionOffset[3];
  double ExclusionOffset[3];

  vtkPVSliceView::VIEW_PLANE Plane;
  vtkPolyDataAlgorithm *Region;
  double Slice;

private:
  vtkRectangularBoundingRegionWidget(const vtkRectangularBoundingRegionWidget&);  //Not implemented
  void operator=(const vtkRectangularBoundingRegionWidget&);  //Not implemented
};

#endif // VTKRECTANGULARBOUNDINGREGIONWIDGET_H