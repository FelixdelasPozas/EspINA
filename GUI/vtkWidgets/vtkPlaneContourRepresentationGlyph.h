#ifndef _VTKPLANECONTOURREPRESENTATIONGLYPH_H_
#define _VTKPLANECONTOURREPRESENTATIONGLYPH_H_

#include "vtkPlaneContourRepresentation.h"
#include "vtkContourToPolygonFilter.h"
#include <QColor>

class vtkProperty;
class vtkActor;
class vtkPolyDataMapper;
class vtkPolyData;
class vtkGlyph3D;
class vtkPoints;

class VTK_WIDGETS_EXPORT vtkPlaneContourRepresentationGlyph
: public vtkPlaneContourRepresentation
{
  public:
    // Description:
    // Instantiate this class.
    static vtkPlaneContourRepresentationGlyph *New();

    // Description:
    // Standard methods for instances of this class.
    vtkTypeMacro(vtkPlaneContourRepresentationGlyph,vtkPlaneContourRepresentation);
    void PrintSelf(ostream& os, vtkIndent indent);

    // Description:
    // Specify the cursor shape. Keep in mind that the shape will be
    // aligned with the  constraining plane by orienting it such that
    // the x axis of the geometry lies along the normal of the plane.
    void SetCursorShape(vtkPolyData *cursorShape);
    vtkPolyData *GetCursorShape();

    // Description:
    // Specify the shape of the cursor (handle) when it is active.
    // This is the geometry that will be used when the mouse is
    // close to the handle or if the user is manipulating the handle.
    void SetActiveCursorShape(vtkPolyData *activeShape);
    vtkPolyData *GetActiveCursorShape();

    // Description:
    // This is the property used when the handle is not active
    // (the mouse is not near the handle)
    vtkGetObjectMacro(Property,vtkProperty);

    // Description:
    // This is the property used when the user is interacting
    // with the handle.
    vtkGetObjectMacro(ActiveProperty,vtkProperty);

    // Description:
    // This is the property used by the lines.
    vtkGetObjectMacro(LinesProperty,vtkProperty);

    // Description:
    // These are the methods that the widget and its representation use to
    // communicate with each other.
    virtual void SetRenderer(vtkRenderer *ren);
    virtual void BuildRepresentation();
    virtual void StartWidgetInteraction(double eventPos[2]);
    virtual void WidgetInteraction(double eventPos[2]);
    virtual int ComputeInteractionState(int X, int Y, int modified = 0);

    // Description:
    // Methods to make this class behave as a vtkProp.
    virtual void GetActors(vtkPropCollection *);
    virtual void ReleaseGraphicsResources(vtkWindow *);
    virtual int RenderOverlay(vtkViewport *viewport);
    virtual int RenderOpaqueGeometry(vtkViewport *viewport);
    virtual int RenderTranslucentPolygonalGeometry(vtkViewport *viewport);
    virtual int HasTranslucentPolygonalGeometry();

    // Description:
    // Get the points in this contour as a vtkPolyData.
    virtual vtkPolyData *GetContourRepresentationAsPolyData();

    // Description:
    // Controls whether the contour widget should always appear on top
    // of other actors in the scene. (In effect, this will disable OpenGL
    // Depth buffer tests while rendering the contour).
    // Default is to set it to false.
    vtkSetMacro(AlwaysOnTop, int);
    vtkGetMacro(AlwaysOnTop, int);
    vtkBooleanMacro( AlwaysOnTop, int );

    // Description:
    // Convenience method to set the line color.
    // Ideally one should use GetLinesProperty()->SetColor().
    void SetLineColor(double r, double g, double b);

    // Description:
    // A flag to indicate whether to show the Selected nodes
    // Default is to set it to false.
    virtual void SetShowSelectedNodes(int);

    // Description:
    // Return the bounds of the representation
    virtual double *GetBounds();

    // spacing is needed for interaction calculations, together with bounds
    vtkSetVector2Macro(Spacing, double);

    double Distance2BetweenPoints(int displayPosX, int displayPosY, int node);

    // get/set polygon color
    virtual void setPolygonColor(QColor);
    virtual QColor getPolygonColor();
  protected:
    vtkPlaneContourRepresentationGlyph();
    ~vtkPlaneContourRepresentationGlyph();

    // Render the cursor
    vtkActor *Actor;
    vtkPolyDataMapper *Mapper;
    vtkGlyph3D *Glypher;
    vtkActor *ActiveActor;
    vtkPolyDataMapper *ActiveMapper;
    vtkGlyph3D *ActiveGlypher;
    vtkPolyData *CursorShape;
    vtkPolyData *ActiveCursorShape;
    vtkPolyData *FocalData;
    vtkPoints *FocalPoint;
    vtkPolyData *ActiveFocalData;
    vtkPoints *ActiveFocalPoint;

    vtkPolyData *SelectedNodesData;
    vtkPoints *SelectedNodesPoints;
    vtkActor *SelectedNodesActor;
    vtkPolyDataMapper *SelectedNodesMapper;
    vtkGlyph3D *SelectedNodesGlypher;
    vtkPolyData *SelectedNodesCursorShape;
    void CreateSelectedNodesRepresentation();

    vtkPolyData *Lines;
    vtkPolyDataMapper *LinesMapper;
    vtkActor *LinesActor;

    // Support picking
    double LastPickPosition[3];
    double LastEventPosition[2];

    // Methods to manipulate the cursor
    void Translate(double eventPos[2]);
    void Scale(double eventPos[2]);
    void ShiftContour(double eventPos[2]);
    void ScaleContour(double eventPos[2]);

    void ComputeCentroid(double* ioCentroid);

    // Properties used to control the appearance of selected objects and
    // the manipulator in general.
    vtkProperty *Property;
    vtkProperty *ActiveProperty;
    vtkProperty *LinesProperty;
    void CreateDefaultProperties();

    // Distance between where the mouse event happens and where the
    // widget is focused - maintain this distance during interaction.
    double InteractionOffset[2];

    int AlwaysOnTop;
    double Spacing[2];

    virtual void BuildLines();

    // toggles the use of a filled polygon as part of the representation
    virtual void UseContourPolygon(bool value);

  private:
    vtkPlaneContourRepresentationGlyph(const vtkPlaneContourRepresentationGlyph&); //Not implemented
    void operator=(const vtkPlaneContourRepresentationGlyph&); //Not implemented

    vtkSmartPointer<vtkActor> m_polygon;
    vtkSmartPointer<vtkContourToPolygonFilter> m_polygonFilter;
    vtkSmartPointer<vtkPolyDataMapper> m_polygonMapper;
    bool useContourPolygon;
    QColor m_polygonColor;
};

#endif // _VTKPLANECONTOURREPRESENTATIONGLYPH_H_
