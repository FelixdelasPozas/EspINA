/*
 * vtkPlanarSplitRepresentation2D.h
 *
 *  Created on: Nov 6, 2012
 *      Author: Félix de las Pozas Álvarez
 */

#ifndef VTKPLANARSPLITREPRESENTATION2D_H_
#define VTKPLANARSPLITREPRESENTATION2D_H_

#include "EspinaGUI_Export.h"

// EspINA
#include <Core/EspinaTypes.h>
#include <Core/Utils/Spatial.h>

// vtk
#include <vtkWidgetRepresentation.h>
#include <vtkSmartPointer.h>
#include <vtkWidgetRepresentation.h>

class vtkPoints;
class vtkLineSource;
class vtkActor;
class vtkPointHandleRepresentation2D;
class vtkHandleRepresentation;

namespace EspINA
{

  class EspinaGUI_EXPORT vtkPlanarSplitRepresentation2D
  : public vtkWidgetRepresentation
  {
    vtkTypeMacro(vtkPlanarSplitRepresentation2D,vtkWidgetRepresentation);

  public:
    static vtkPlanarSplitRepresentation2D *New();

    vtkSmartPointer<vtkPoints> getPoints();
    void setPoints(vtkSmartPointer<vtkPoints>);
    void setPoint1(Nm *);
    void setPoint2(Nm *);
    void getPoint1(Nm *);
    void getPoint2(Nm *);


    // Description:
    // The tolerance representing the distance to the widget (in pixels) in
    // which the cursor is considered near enough to the end points of
    // the widget to be active.
    vtkSetClampMacro(m_tolerance,int,1,100);
    vtkGetMacro(m_tolerance,int);

    // Description:
    // These are methods that satisfy vtkWidgetRepresentation's API.
    virtual void BuildRepresentation();
    virtual int ComputeInteractionState(int X, int Y, int modify=0);
    virtual void StartWidgetInteraction(double e[2]);
    virtual void WidgetInteraction(double e[2]);

    // Description:
    // Methods required by vtkProp superclass.
    virtual void ReleaseGraphicsResources(vtkWindow *w);
    virtual int RenderOverlay(vtkViewport *viewport);
    virtual int RenderOpaqueGeometry(vtkViewport *viewport);

    enum { Outside = 0, NearP1, NearP2 };

    // Description:
    // This method is used to specify the type of handle representation to
    // use for the two internal vtkHandleWidgets within the widget.
    // To use this method, create a dummy vtkHandleWidget (or subclass),
    // and then invoke this method with this dummy. Then the
    // representation uses this dummy to clone two vtkHandleWidgets
    // of the same type. Make sure you set the handle representation before
    // the widget is enabled. (The method InstantiateHandleRepresentation()
    // is invoked by the widget.)
    void SetHandleRepresentation(vtkHandleRepresentation *handle);
    void InstantiateHandleRepresentation();
    void MoveHandle(int handleNum, int X, int Y);

    // Description:
    // Set/Get the two handle representations used for the widget. (Note:
    // properties can be set by grabbing these representations and setting the
    // properties appropriately.)
    vtkGetObjectMacro(Point1Representation,vtkHandleRepresentation);
    vtkGetObjectMacro(Point2Representation,vtkHandleRepresentation);

    // set representation orientation
    virtual void setOrientation(Plane orientation);

    // set segmentation bounds to draw the actor
    virtual void setSegmentationBounds(double *);
    virtual void removeBoundsActor();

  protected:
    vtkPlanarSplitRepresentation2D();
    ~vtkPlanarSplitRepresentation2D();

    double m_point1[3];
    double m_point2[3];
    vtkSmartPointer<vtkLineSource> m_line;
    vtkSmartPointer<vtkActor> m_lineActor;
    vtkSmartPointer<vtkPoints> m_boundsPoints;
    vtkSmartPointer<vtkActor> m_boundsActor;

    // definition and clones
    vtkHandleRepresentation *HandleRepresentation;
    vtkHandleRepresentation *Point1Representation;
    vtkHandleRepresentation *Point2Representation;

    // Selection tolerance for the handles
    int    m_tolerance;
    Plane  m_plane;
    double m_epsilon;
  private:
    vtkPlanarSplitRepresentation2D(const vtkPlanarSplitRepresentation2D&);  //Not implemented
    void operator=(const vtkPlanarSplitRepresentation2D&);  //Not implemented
  };

} // namespace EspINA

#endif /* VTKPLANARSPLITREPRESENTATION2D_H_ */
