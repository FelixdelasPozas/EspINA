#ifndef VTKRECTANGULARSLICEREPRESENTATION_H
#define VTKRECTANGULARSLICEREPRESENTATION_H

#include "GUI/EspinaGUI_Export.h"

#include "vtkWidgetRepresentation.h"

#include <Core/EspinaTypes.h>
#include <Core/Utils/Spatial.h>

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

namespace EspINA
{

class EspinaGUI_EXPORT vtkRectangularSliceRepresentation
: public vtkWidgetRepresentation
{
  //BTX
  enum EDGE {LEFT, TOP, RIGHT, BOTTOM};
  //ETX

public:
  // Description:
  // Instantiate the class.
  static vtkRectangularSliceRepresentation *New();

  // Description:
  // Standard methods for the class.
  vtkTypeMacro(vtkRectangularSliceRepresentation,vtkWidgetRepresentation);
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
  virtual void SetPlane(Plane plane);
  virtual void SetSlice(Nm pos);
  virtual void SetCuboidBounds(double bounds[6]);
  virtual void GetCuboidBounds(double bounds[6]);

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
  enum {Outside=0, Inside,
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

  // modify representation methods
  void setRepresentationColor(double *);
  void setRepresentationPattern(int);

protected:
  vtkRectangularSliceRepresentation();
  ~vtkRectangularSliceRepresentation();

  // Manage how the representation appears
  double LastEventPosition[3];

  // Counting Region Edge
  vtkActor 	    *EdgeActor[4];
  vtkPolyDataMapper *EdgeMapper[4];
  vtkPolyData 	    *EdgePolyData[4];
  vtkPoints	    *Vertex;

  void HighlightEdge(vtkActor *actor);
  void Highlight();

  // Do the picking
  vtkCellPicker *EdgePicker;
  vtkCellPicker *LastPicker;
  vtkActor *CurrentEdge;

  // Properties used to control the appearance of selected objects and
  // the manipulator in general.
  vtkProperty *EdgeProperty;
  vtkProperty *SelectedEdgeProperty;
  vtkProperty *InvisibleProperty;

  virtual void CreateDefaultProperties();

  int hCoord() const {return Plane::YZ == m_plane?2:0;}
  int vCoord() const {return Plane::XZ == m_plane?2:1;}

  // Helper methods to create face representations
  virtual void CreateRegion();
  virtual void UpdateRegion();
  virtual void UpdateXYFace();
  virtual void UpdateYZFace();
  virtual void UpdateXZFace();

  // Helper methods
  void MoveLeftEdge(double *p1, double *p2);
  void MoveRightEdge(double *p1, double *p2);
  void MoveTopEdge(double *p1, double *p2);
  void MoveBottomEdge(double *p1, double *p2);
  void Translate(double *p1, double *p2);

  Plane m_plane;
  Nm Slice;

private:
  vtkRectangularSliceRepresentation(const vtkRectangularSliceRepresentation&);  //Not implemented
  void operator=(const vtkRectangularSliceRepresentation&);  //Not implemented

  double m_bounds[6];

  int NumPoints;
  int NumSlices;
  int NumVertex;

  double m_repBounds[6];

  double LeftEdge;
  double TopEdge;
  double RightEdge;
  double BottomEdge;

  double m_color[3];
  int m_pattern;
};

} // namespace EspINA

#endif
