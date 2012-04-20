#ifndef VTKRECTANGULARBOUNDINGBOXREPRESENTATION_H
#define VTKRECTANGULARBOUNDINGBOXREPRESENTATION_H

#include "vtkWidgetRepresentation.h"
#include <common/views/vtkPVSliceView.h>

class vtkLookupTable;
class vtkPolyDataAlgorithm;
class vtkActor;
class vtkPolyDataMapper;
class vtkLineSource;
class vtkSphereSource;
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


class VTK_WIDGETS_EXPORT vtkRectangularBoundingRegionRepresentation : public vtkWidgetRepresentation
{
  //BTX
  enum EDGE {LEFT, TOP, RIGHT, BOTTOM};
  //ETX

public:
  // Description:
  // Instantiate the class.
  static vtkRectangularBoundingRegionRepresentation *New();

  // Description:
  // Standard methods for the class.
  vtkTypeMacro(vtkRectangularBoundingRegionRepresentation,vtkWidgetRepresentation);
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
  vtkGetObjectMacro(SelectedEdgeProperty,vtkProperty);

  // Description:
  // Get the view type properties. In which plane it is been shown
  // and which slice (in case of planar views) is selected
//   vtkSetMacro(ViewType,int);
//   vtkSetMacro(Slice,int);
  virtual void SetPlane(vtkPVSliceView::VIEW_PLANE plane);
  virtual void SetSlice(double pos);
  virtual void SetRegion(vtkPolyDataAlgorithm *region);
  
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
  vtkRectangularBoundingRegionRepresentation();
  ~vtkRectangularBoundingRegionRepresentation();

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

  // Support GetBounds() method
  vtkBox *BoundingBox;

  // Properties used to control the appearance of selected objects and
  // the manipulator in general.
  vtkProperty *InclusionEdgeProperty;
  vtkProperty *ExclusionEdgeProperty;
  vtkProperty *SelectedEdgeProperty;
  vtkProperty *InvisibleProperty;

  virtual void CreateDefaultProperties();

  // Helper methods to create face representations
  virtual void CreateRegion();
  virtual void UpdateRegion();
  virtual void CreateXYFace();
  virtual void UpdateXYFace();
  virtual void CreateYZFace();
  virtual void UpdateYZFace();
  virtual void CreateXZFace();
  virtual void UpdateXZFace();

  // Helper methods
  void MoveLeftEdge(double *p1, double *p2);
  void MoveRightEdge(double *p1, double *p2);
  void MoveTopEdge(double *p1, double *p2);
  void MoveBottomEdge(double *p1, double *p2);
  void MoveUpperEdge(double *p1, double *p2);
  void MoveLowerEdge(double *p1, double *p2);

  vtkPVSliceView::VIEW_PLANE Plane;
  vtkPolyDataAlgorithm *Region;
  double Slice;
  bool Init;


private:
  vtkRectangularBoundingRegionRepresentation(const vtkRectangularBoundingRegionRepresentation&);  //Not implemented
  void operator=(const vtkRectangularBoundingRegionRepresentation&);  //Not implemented

  double InclusionOffset[3];
  double ExclusionOffset[3];

  int m_numSlices;
  double m_prevInclusion[3];
  double m_prevExclusion[3];
  double m_lastInclusionEdge[3];
  double m_lastExclusionEdge[3];
};

#endif