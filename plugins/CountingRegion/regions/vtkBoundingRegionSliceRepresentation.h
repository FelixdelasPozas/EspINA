#ifndef VTKRECTANGULARBOUNDINGBOXREPRESENTATION_H
#define VTKRECTANGULARBOUNDINGBOXREPRESENTATION_H

#include "vtkWidgetRepresentation.h"
#include <vtkSmartPointer.h>
#include <vtkPolyData.h>

#include <common/EspinaTypes.h>

class vtkLookupTable;
class vtkPolyDataAlgorithm;
class vtkActor;
class vtkPolyDataMapper;
class vtkLineSource;
class vtkCellPicker;
class vtkProperty;
class vtkPolyData;
class vtkPoints;
class vtkPolyDataAlgorithm;
class vtkPointHandleRepresentation3D;
class vtkTransform;
class vtkPlanes;
class vtkBox;
class vtkDoubleArray;
class vtkMatrix4x4;

class VTK_WIDGETS_EXPORT vtkBoundingRegionSliceRepresentation
: public vtkWidgetRepresentation
{
  //BTX
  enum EDGE {LEFT, TOP, RIGHT, BOTTOM};
  //ETX

public:
  // Description:
  // Instantiate the class.
  static vtkBoundingRegionSliceRepresentation *New();

  // Description:
  // Standard methods for the class.
  vtkTypeMacro(vtkBoundingRegionSliceRepresentation,vtkWidgetRepresentation);
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
  // Get the outline properties (the outline of the box). The 
  // properties of the outline when selected and normal can be 
  // set.
//   vtkGetObjectMacro(OutlineProperty,vtkProperty);
  vtkGetObjectMacro(SelectedInclusionProperty,vtkProperty);

  // Description:
  // Get the view type properties. In which plane it is been shown
  // and which slice (in case of planar views) is selected
//   vtkSetMacro(ViewType,int);
//   vtkSetMacro(Slice,int);
  virtual void SetPlane(PlaneType plane);
  virtual void SetSlice(Nm pos);
  virtual void SetBoundingRegion(vtkSmartPointer<vtkPolyData> region,
                                 Nm slicingStep[3]);

  // Description:
  // These are methods to communicate with the 3d_widget
  vtkSetVector3Macro(InclusionOffset, double);
  vtkGetVector3Macro(InclusionOffset, double);
  vtkSetVector3Macro(ExclusionOffset, Nm);
  vtkGetVector3Macro(ExclusionOffset, Nm);

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
    MoveLeft, MoveRight, MoveTop, MoveBottom, Translating
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
  vtkBoundingRegionSliceRepresentation();
  ~vtkBoundingRegionSliceRepresentation();

  // Manage how the representation appears
  double LastEventPosition[3];

  // Counting Region Edge
  vtkActor 	    *EdgeActor[4];
  vtkPolyDataMapper *EdgeMapper[4];
  vtkPolyData 	    *EdgePolyData[4];
  vtkPoints	    *Vertex;

  void HighlightEdge(vtkActor *actor);

  // Do the picking
  vtkCellPicker *EdgePicker;
  vtkCellPicker *LastPicker;
  vtkActor *CurrentEdge;

  // Properties used to control the appearance of selected objects and
  // the manipulator in general.
  vtkProperty *InclusionEdgeProperty;
  vtkProperty *ExclusionEdgeProperty;
  vtkProperty *SelectedInclusionProperty;
  vtkProperty *SelectedExclusionProperty;
  vtkProperty *InvisibleProperty;

  virtual void CreateDefaultProperties();

  int hCoord() const {return SAGITTAL == Plane?2:0;}
  int vCoord() const {return CORONAL  == Plane?2:1;}
  double leftEdge() {return GetBounds()[hCoord()*2] + Shift[LEFT];}
  double topEdge() {return GetBounds()[vCoord()*2] + Shift[TOP];}
  double rightEdge() {return GetBounds()[hCoord()*2+1] + Shift[RIGHT];}
  double bottomEdge() {return GetBounds()[vCoord()*2+1] + Shift[BOTTOM];}

  int sliceNumber(Nm pos, PlaneType plane) const;

  // Helper methods to create face representations
  virtual void CreateRegion();
  virtual void UpdateRegion();
  virtual void CreateXYFace();
  virtual void UpdateXYFace();
  virtual void CreateYZFace();
  virtual void CreateXZFace();

  // Helper methods
  void MoveLeftEdge(double *p1, double *p2);
  void MoveRightEdge(double *p1, double *p2);
  void MoveTopEdge(double *p1, double *p2);
  void MoveBottomEdge(double *p1, double *p2);

  PlaneType Plane;
  vtkSmartPointer<vtkPolyData> Region;
  Nm Slice;
  Nm SlicingStep[3];
  double Shift[4];
  bool Init;

private:
  vtkBoundingRegionSliceRepresentation(const vtkBoundingRegionSliceRepresentation&);  //Not implemented
  void operator=(const vtkBoundingRegionSliceRepresentation&);  //Not implemented

  Nm InclusionOffset[3];
  Nm ExclusionOffset[3];

  int NumPoints;
  int NumSlices;
  int NumVertex;

  double RepBounds[6];
};

#endif