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

#include "CountingFramePlugin_Export.h"

#include "vtkWidgetRepresentation.h"
#include <vtkSmartPointer.h>
#include <vtkPolyData.h>

#include <Core/EspinaTypes.h>

class vtkLookupTable;
class vtkPolyDataAlgorithm;
class vtkActor;
class vtkPolyDataMapper;
class vtkLineSource;
class vtkCellPicker;
class vtkProperty;
class vtkPoints;
class vtkPolyDataAlgorithm;
class vtkPointHandleRepresentation3D;
class vtkPlanes;
class vtkBox;
class vtkDoubleArray;
class vtkMatrix4x4;


class CountingFramePlugin_EXPORT vtkCountingFrame3DRepresentation
: public vtkWidgetRepresentation
{
  //BTX
  enum MARGIN {LEFT, TOP, UPPER, RIGHT, BOTTOM, LOWER, VOLUME};
  //ETX
public:
  // Description:
  // Instantiate the class.
  static vtkCountingFrame3DRepresentation *New();

  // Description:
  // Standard methods for the class.
  vtkTypeMacro(vtkCountingFrame3DRepresentation, vtkWidgetRepresentation);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Grab the polydata (including points) that define the box widget. The
  // polydata consists of 6 quadrilateral faces and 15 points. The first
  // eight points define the eight corner vertices; the next six define the
  // -x,+x, -y,+y, -z,+z face points; and the final point (the 15th out of 15
  // points) defines the center of the box. These point values are guaranteed
  // to be up-to-date when either the widget's corresponding InteractionEvent
  // or EndInteractionEvent events are invoked. The user provides the
  // vtkPolyData and the points and cells are added to it.
  void GetPolyData(vtkPolyData *pd);

  void reset();

  // Description:
  // Get the face properties (the faces of the box). The 
  // properties of the face when selected and normal can be 
  // set.
  vtkGetObjectMacro(FaceProperty,vtkProperty);
  vtkGetObjectMacro(SelectedFaceProperty,vtkProperty);

  // Description:
  // Get the outline properties (the outline of the box). The 
  // properties of the outline when selected and normal can be 
  // set.
  vtkGetObjectMacro(SelectedOutlineProperty,vtkProperty);

  virtual void SetCountingFrame(vtkSmartPointer<vtkPolyData> region);

  // Description:
  // These are methods to communicate with the 3d_widget
  vtkSetVector3Macro(InclusionOffset, double);
  vtkGetVector3Macro(InclusionOffset, double);
  vtkSetVector3Macro(ExclusionOffset, double);
  vtkGetVector3Macro(ExclusionOffset, double);

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
    MoveLeft, MoveRight, MoveTop, MoveBottom, MoveUpper, MoveLower,
  };
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
  vtkCountingFrame3DRepresentation();
  ~vtkCountingFrame3DRepresentation();

  // Manage how the representation appears
  double LastEventPosition[3];

  vtkLookupTable *InclusionLUT;

  // Inclusin Margin
  vtkActor 	    *MarginActor[7]; 
  vtkPolyDataMapper *MarginMapper[7];
  vtkPolyData 	    *MarginPolyData[7];
  vtkPoints	    *MarginPoints;

  // 3D Volume
  vtkActor 	    *VolumeActor;    // Exclusion Actor
  vtkPolyDataMapper *VolumeMapper;
  vtkPolyData 	    *VolumePolyData;
  vtkPoints	    *VolumePoints;

  void HighlightMargin(vtkActor *actor);
//   void HighlightFace(int cellId);
  void HighlightOutline(int highlight);

  // Do the picking
  vtkCellPicker *VolumePicker;
  vtkActor *CurrentHandle;
  int      CurrentHexFace;
  vtkCellPicker *LastPicker;

  // Support GetBounds() method
  vtkBox *BoundingBox;

  // Properties used to control the appearance of selected objects and
  // the manipulator in general.
  vtkProperty *FaceProperty;
  vtkProperty *SelectedFaceProperty;
  vtkProperty *InclusionProperty;
  vtkProperty *InvisibleProperty;
  vtkProperty *SelectedOutlineProperty;
  virtual void CreateDefaultProperties();

  // Helper methods to create face representations
  virtual void CreateVolume();

  // Helper methods
  void MoveLeftMargin(double *p1, double *p2);
  void MoveRightMargin(double *p1, double *p2);
  void MoveTopMargin(double *p1, double *p2);
  void MoveBottomMargin(double *p1, double *p2);
  void MoveUpperMargin(double *p1, double *p2);
  void MoveLowerMargin(double *p1, double *p2);

  vtkSmartPointer<vtkPolyData> CountingFrame;

private:
  vtkCountingFrame3DRepresentation(const vtkCountingFrame3DRepresentation&);  //Not implemented
  void operator=(const vtkCountingFrame3DRepresentation&);  //Not implemented

  double InclusionOffset[3];
  double ExclusionOffset[3];

  double m_prevInclusion[3];
  double m_prevExclusion[3];
  double m_lastInclusionMargin[3];
  double m_lastExclusionMargin[3];
};

#endif
