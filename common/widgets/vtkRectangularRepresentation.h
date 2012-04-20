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

#ifndef VTKRECTANGULARREPRESENTATION_H
#define VTKRECTANGULARREPRESENTATION_H

#include "vtkWidgetRepresentation.h"
#include <vtkPVSliceView.h>

class vtkActor;
class vtkBox;
class vtkCellPicker;
class vtkPoints;
class vtkPolyData;
class vtkPolyDataMapper;
class vtkProperty;

class VTK_WIDGETS_EXPORT vtkRectangularRepresentation
: public vtkWidgetRepresentation
{
  //BTX
  enum EDGE {TOP, RIGHT, BOTTOM, LEFT};
  //ETX
public:
  // Description:
  // Instantiate the class.
  static vtkRectangularRepresentation *New();

  // Description:
  // Standard methods for the class.
  vtkTypeMacro(vtkRectangularRepresentation,vtkWidgetRepresentation);
  void PrintSelf(ostream& os, vtkIndent indent);

  void reset();

  // Description:
  // Get the outline properties (the outline of the box). The 
  // properties of the outline when selected and normal can be 
  // set.
//   vtkGetObjectMacro(OutlineProperty,vtkProperty);
  vtkGetObjectMacro(SelectedEdgeProperty,vtkProperty);

  // Description:
  // Get the view type properties. In which plane it is been shown
  // and which slice (in case of planar views) is selected
  virtual void SetPlane(vtkPVSliceView::VIEW_PLANE plane);

  // Description:
  // These are methods that satisfy vtkWidgetRepresentation's API.
  virtual void PlaceWidget(double bounds[6]);
  virtual void BuildRepresentation();
  virtual int  ComputeInteractionState(int X, int Y, int modify=0);
  virtual void StartWidgetInteraction(double e[2]);
  virtual void WidgetInteraction(double e[2]);
  virtual double *GetBounds();

  // Description:
  // Methods supporting, and required by, the rendering process.
  virtual void ReleaseGraphicsResources(vtkWindow*);
  virtual int  RenderOpaqueGeometry(vtkViewport*);
  virtual int  RenderTranslucentPolygonalGeometry(vtkViewport*);
  virtual int  HasTranslucentPolygonalGeometry();

//BTX - used to manage the state of the widget
  enum {Outside=0,
    MoveLeft, MoveRight, MoveTop, MoveBottom,
    Translating,Scaling};
//ETX

  // Description:
  // The interaction state may be set from a widget (e.g., vtkBoxWidget2) or
  // other object. This controls how the interaction with the widget
  // proceeds. Normally this method is used as part of a handshaking
  // process with the widget: First ComputeInteractionState() is invoked that
  // returns a state based on geometric considerations (i.e., cursor near a
  // widget feature), then based on events, the widget may modify this
  // further.
  void SetInteractionState(int state);

protected:
  vtkRectangularRepresentation();
  ~vtkRectangularRepresentation();

  // Manage how the representation appears
  double LastEventPosition[3];

  // Rectangle's edges
  vtkActor 	    *EdgeActor[4];
  vtkPolyDataMapper *EdgeMapper[4];
  vtkPolyData 	    *EdgePolyData[4];
  vtkPoints	    *Vertex;

  // Rectangular Region
  vtkActor 	    *RegionActor;
  vtkPolyDataMapper *RegionMapper;
  vtkPolyData 	    *RegionPolyData;

  void HighlightEdge(vtkActor *actor);
  void HighlightOutline(int highlight);

  // Do the picking
  vtkCellPicker *EdgePicker;
  vtkCellPicker *RegionPicker;
  vtkCellPicker *LastPicker;
  vtkActor *CurrentHandle;

  // Support GetBounds() method
  vtkBox *BoundingBox;

  // Properties used to control the appearance of selected objects and
  // the manipulator in general.
  vtkProperty *EdgeProperty;
  vtkProperty *RegionProperty;
  vtkProperty *SelectedEdgeProperty;
  vtkProperty *InvisibleProperty;

  virtual void CreateDefaultProperties();

  // Helper functions to get bounds according to to plane
  int leftIndex() const   {return Plane==vtkPVSliceView::SAGITTAL?4:0;}
  int rightIndex() const  {return Plane==vtkPVSliceView::SAGITTAL?5:1;}
  int topIndex() const    {return Plane==vtkPVSliceView::CORONAL?4:2;}
  int bottomIndex() const {return Plane==vtkPVSliceView::CORONAL?5:3;}

  int hCoord() const {return Plane==vtkPVSliceView::SAGITTAL?2:0;}
  int vCoord() const {return Plane==vtkPVSliceView::CORONAL?2:1;}

  // Helper method to place every vertex according to Plane
  virtual void updateVertex();

  // Helper methods
  virtual void Translate(double *p1, double *p2);
  virtual void Scale(double *p1, double *p2, int X, int Y);
  // Directions are relative to the plane selected
  void MoveLeftEdge(double *p1, double *p2);
  void MoveRightEdge(double *p1, double *p2);
  void MoveTopEdge(double *p1, double *p2);
  void MoveBottomEdge(double *p1, double *p2);

  vtkPVSliceView::VIEW_PLANE Plane;

private:
  vtkRectangularRepresentation(const vtkRectangularRepresentation&);  //Not implemented
  void operator=(const vtkRectangularRepresentation&);  //Not implemented
};

#endif