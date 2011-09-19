#ifndef VTKRECTANGULARBOUNDINGBOXREPRESENTATION_H
#define VTKRECTANGULARBOUNDINGBOXREPRESENTATION_H

#include "vtkWidgetRepresentation.h"

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
public:
  // Description:
  // Instantiate the class.
  static vtkRectangularBoundingRegionRepresentation *New();

  // Description:
  // Standard methods for the class.
  vtkTypeMacro(vtkRectangularBoundingRegionRepresentation,vtkWidgetRepresentation);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Retrieve a linear transform characterizing the transformation of the
  // box. Note that the transformation is relative to where PlaceWidget()
  // was initially called. This method modifies the transform provided. The
  // transform can be used to control the position of vtkProp3D's, as well as
  // other transformation operations (e.g., vtkTranformPolyData).
  virtual void GetTransform(vtkTransform *t);

  // Description:
  // Set the position, scale and orientation of the box widget using the
  // transform specified. Note that the transformation is relative to 
  // where PlaceWidget() was initially called (i.e., the original bounding
  // box). 
  virtual void SetTransform(vtkTransform* t);

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
//   vtkGetObjectMacro(OutlineProperty,vtkProperty);
  vtkGetObjectMacro(SelectedOutlineProperty,vtkProperty);

  // Description:
  // Get the view type properties. In which plane it is been shown
  // and which slice (in case of planar views) is selected
//   vtkSetMacro(ViewType,int);
//   vtkSetMacro(Slice,int);
  virtual void SetViewType(int type);
  virtual void SetSlice(int slice, double spacing);
  virtual void SetRegion(vtkPolyDataAlgorithm *region);
  
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
  enum {Outside=0,MoveF0,MoveF1,MoveF2,MoveF3,MoveF4,MoveF5,Translating,Rotating,Scaling};
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
  
  vtkPoints         *Points;  //used by others as well
  double             N[6][3]; //the normals of the faces
  
  // bounding region
  vtkActor          *InclusionActor;
  vtkPolyDataMapper *InclusionMapper;
  vtkPolyData       *InclusionPolyData;
  vtkLookupTable    *InclusionLUT;

  // bounding face (4 lines)
  vtkPoints         *BoundingFacePoints;
  vtkPolyDataMapper *BoundingFaceMapper;
  vtkPolyData       *BoundingFacePolyData;

  // A face of the hexahedron
  vtkActor          *HexFace;
  vtkPolyDataMapper *HexFaceMapper;
  vtkPolyData       *HexFacePolyData;

//   int HighlightHandle(vtkProp *prop); //returns cell id
  void HighlightFace(int cellId);
  void HighlightOutline(int highlight);
  virtual void ComputeNormals();
  
  // Do the picking
  vtkCellPicker *HexPicker;
  vtkActor *CurrentHandle;
  int      CurrentHexFace;
  vtkCellPicker *LastPicker;
  
  // Transform the hexahedral points (used for rotations)
  vtkTransform *Transform;
  
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
  virtual void CreateXYFace();
  virtual void CreateYZFace();
  virtual void CreateXZFace();
  
  
  // Helper methods
  virtual void Translate(double *p1, double *p2);
  virtual void Scale(double *p1, double *p2, int X, int Y);
  void MovePlusXFace(double *p1, double *p2);
  void MoveMinusXFace(double *p1, double *p2);
  void MovePlusYFace(double *p1, double *p2);
  void MoveMinusYFace(double *p1, double *p2);
  void MovePlusZFace(double *p1, double *p2);
  void MoveMinusZFace(double *p1, double *p2);

  // Internal ivars for performance
  vtkPoints      *PlanePoints;
  vtkDoubleArray *PlaneNormals;
  vtkMatrix4x4   *Matrix;

  //"dir" is the direction in which the face can be moved i.e. the axis passing
  //through the center
  void MoveFace(double *p1, double *p2, double *dir, 
                double *x1, double *x2, double *x3, double *x4,
                double *x5);
  //Helper method to obtain the direction in which the face is to be moved.
  //Handles special cases where some of the scale factors are 0.
  void GetDirection(const double Nx[3],const double Ny[3], 
                    const double Nz[3], double dir[3]);
  
  int ViewType;
  int Slice;
  double Spacing;
  vtkPolyDataAlgorithm *Region;


private:
  vtkRectangularBoundingRegionRepresentation(const vtkRectangularBoundingRegionRepresentation&);  //Not implemented
  void operator=(const vtkRectangularBoundingRegionRepresentation&);  //Not implemented
};

#endif