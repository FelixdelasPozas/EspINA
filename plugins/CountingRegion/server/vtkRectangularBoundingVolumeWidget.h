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

#ifndef VTKRECTANGULARBOUNDINGVOLUMEWIDGET_H
#define VTKRECTANGULARBOUNDINGVOLUMEWIDGET_H

#include <vtkAbstractWidget.h>

#include <common/views/vtkPVSliceView.h>

class vtkPolyDataAlgorithm;
class vtkRectangularBoundingVolumeRepresentation;

class VTK_WIDGETS_EXPORT vtkRectangularBoundingVolumeWidget : public vtkAbstractWidget
{
public:
  // Description:
  // Instantiate the object.
  static vtkRectangularBoundingVolumeWidget *New();

  // Description:
  // Standard class methods for type information and printing.
  vtkTypeMacro(vtkRectangularBoundingVolumeWidget,vtkAbstractWidget);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Specify an instance of vtkWidgetRepresentation used to represent this
  // widget in the scene. Note that the representation is a subclass of vtkProp
  // so it can be added to the renderer independent of the widget.
  void SetRepresentation(vtkRectangularBoundingVolumeRepresentation *r)
    {this->Superclass::SetWidgetRepresentation(reinterpret_cast<vtkWidgetRepresentation*>(r));}

  vtkSetVector3Macro(InclusionOffset, double);
  vtkGetVector3Macro(InclusionOffset, double);
  vtkSetVector3Macro(ExclusionOffset, double);
  vtkGetVector3Macro(ExclusionOffset, double);

  virtual void SetVolume(vtkPolyDataAlgorithm *region);

  // Description:
  // Create the default widget representation if one is not set. By default,
  // this is an instance of the vtkRectangularBoundingVolumeRepresentation class.
  void CreateDefaultRepresentation();

protected:
  vtkRectangularBoundingVolumeWidget();
  ~vtkRectangularBoundingVolumeWidget();

//BTX - manage the state of the widget
  int WidgetState;
  enum _WidgetState {Start=0,Active};
//ETX

  // These methods handle events
  static void SelectAction(vtkAbstractWidget*);
  static void EndSelectAction(vtkAbstractWidget*);

  // helper methods for cursoe management
  virtual void SetCursor(int state);

  double InclusionOffset[3];
  double ExclusionOffset[3];

  vtkPolyDataAlgorithm *Volume;

private:
  vtkRectangularBoundingVolumeWidget(const vtkRectangularBoundingVolumeWidget&);  //Not implemented
  void operator=(const vtkRectangularBoundingVolumeWidget&);  //Not implemented
};

#endif
