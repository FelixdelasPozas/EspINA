#ifndef VTKRECTANGULARBOUNDINGBOXREPRESENTATION_H
#define VTKRECTANGULARBOUNDINGBOXREPRESENTATION_H

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
protected:
  //BTX
  enum EDGE {LEFT, TOP, RIGHT, BOTTOM};
  //ETX

public:
  // Description:
  // Standard methods for the class.
  vtkTypeMacro(vtkBoundingRegionSliceRepresentation,vtkWidgetRepresentation);
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
  virtual void SetSlice(Nm pos);
  virtual void SetBoundingRegion(vtkSmartPointer<vtkPolyData> region,
                                 Nm inclusionOffset[3],
                                 Nm exclusionOffset[3],
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

  void regionBounds(int regionSlice, Nm bounds[6]);

  virtual Nm realLeftEdge  (int slice=0) = 0;
  virtual Nm realTopEdge   (int slice=0) = 0;
  virtual Nm realRightEdge (int slice=0) = 0;
  virtual Nm realBottomEdge(int slice=0) = 0;

  virtual Nm leftEdge  (int slice=0) = 0;
  virtual Nm topEdge   (int slice=0) = 0;
  virtual Nm rightEdge (int slice=0) = 0;
  virtual Nm bottomEdge(int slice=0) = 0;

  /// @pos in Z dir
  int sliceNumber(Nm pos) const;

  // Helper methods to create face representations
  virtual void CreateRegion() = 0;

  // Helper methods
  virtual void MoveLeftEdge  (double *p1, double *p2) = 0;
  virtual void MoveRightEdge (double *p1, double *p2) = 0;
  virtual void MoveTopEdge   (double *p1, double *p2) = 0;
  virtual void MoveBottomEdge(double *p1, double *p2) = 0;

protected:
  vtkSmartPointer<vtkPolyData> Region;
  Nm Slice;
  Nm Resolution[3];

  bool Init;
  Nm InclusionOffset[3];
  Nm ExclusionOffset[3];

  int NumPoints;
  int NumSlices;
  int NumVertex;

private:
  vtkBoundingRegionSliceRepresentation(const vtkBoundingRegionSliceRepresentation&);  //Not implemented
  void operator=(const vtkBoundingRegionSliceRepresentation&);  //Not implemented
};

#endif