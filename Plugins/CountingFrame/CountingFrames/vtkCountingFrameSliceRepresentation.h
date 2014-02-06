#ifndef VTKRECTANGULARBOUNDINGBOXREPRESENTATION_H
#define VTKRECTANGULARBOUNDINGBOXREPRESENTATION_H

#include "CountingFramePlugin_Export.h"

#include "vtkWidgetRepresentation.h"
#include <vtkSmartPointer.h>
#include <vtkPolyData.h>

#include <Core/Utils/NmVector3.h>

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

class CountingFramePlugin_EXPORT vtkCountingFrameSliceRepresentation
: public vtkWidgetRepresentation
{
protected:
  //BTX
  enum EDGE {LEFT, TOP, RIGHT, BOTTOM};
  //ETX

public:
  // Description:
  // Standard methods for the class.
  vtkTypeMacro(vtkCountingFrameSliceRepresentation,vtkWidgetRepresentation);
  void PrintSelf(ostream& os, vtkIndent indent);

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
  virtual void SetSlice(EspINA::Nm pos);
  virtual void SetCountingFrame(vtkSmartPointer<vtkPolyData> region,
                                 EspINA::Nm inclusionOffset[3],
                                 EspINA::Nm exclusionOffset[3],
                                 EspINA::NmVector3 slicingStep);


  virtual void SetHighlighted(bool highlight);

  // Description:
  // These are methods to communicate with the 3d_widget
  vtkSetVector3Macro(InclusionOffset, double);
  vtkGetVector3Macro(InclusionOffset, double);
  vtkSetVector3Macro(ExclusionOffset, EspINA::Nm);
  vtkGetVector3Macro(ExclusionOffset, EspINA::Nm);

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
  vtkCountingFrameSliceRepresentation();
  ~vtkCountingFrameSliceRepresentation();

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

  void regionBounds(int regionSlice, EspINA::Nm bounds[6]);

  virtual EspINA::Nm realLeftEdge  (int slice=0) = 0;
  virtual EspINA::Nm realTopEdge   (int slice=0) = 0;
  virtual EspINA::Nm realRightEdge (int slice=0) = 0;
  virtual EspINA::Nm realBottomEdge(int slice=0) = 0;

  virtual EspINA::Nm leftEdge  (int slice=0) = 0;
  virtual EspINA::Nm topEdge   (int slice=0) = 0;
  virtual EspINA::Nm rightEdge (int slice=0) = 0;
  virtual EspINA::Nm bottomEdge(int slice=0) = 0;

  /// @pos in Z dir
  int sliceNumber(EspINA::Nm pos) const;

  // Helper methods to create face representations
  virtual void CreateRegion() = 0;

  // Helper methods
  virtual void MoveLeftEdge  (double *p1, double *p2) = 0;
  virtual void MoveRightEdge (double *p1, double *p2) = 0;
  virtual void MoveTopEdge   (double *p1, double *p2) = 0;
  virtual void MoveBottomEdge(double *p1, double *p2) = 0;

protected:
  vtkSmartPointer<vtkPolyData> Region;
  EspINA::Nm Slice;
  EspINA::NmVector3 SlicingStep;

  bool Init;
  EspINA::Nm InclusionOffset[3];
  EspINA::Nm ExclusionOffset[3];

  int NumPoints;
  int NumSlices;
  int NumVertex;

private:
  vtkCountingFrameSliceRepresentation(const vtkCountingFrameSliceRepresentation&);  //Not implemented
  void operator=(const vtkCountingFrameSliceRepresentation&);  //Not implemented
};

#endif
