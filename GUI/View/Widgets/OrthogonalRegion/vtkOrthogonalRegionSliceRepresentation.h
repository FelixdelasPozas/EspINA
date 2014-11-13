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

#ifndef ESPINA_VTK_ORTHOGONAL_REGION_SLICE_REPRESENTATION_H
#define ESPINA_VTK_ORTHOGONAL_REGION_SLICE_REPRESENTATION_H

#include "GUI/EspinaGUI_Export.h"

// ESPINA
#include <Core/EspinaTypes.h>
#include <Core/Utils/Spatial.h>

// VTK
#include <vtkSmartPointer.h>
#include "vtkWidgetRepresentation.h"

class vtkActor;
class vtkPolyDataMapper;
class vtkCellPicker;
class vtkProperty;
class vtkPolyData;
class vtkPoints;

namespace ESPINA
{
  class View2D;

  class EspinaGUI_EXPORT vtkOrthogonalRegionSliceRepresentation
  : public vtkWidgetRepresentation
  {
    //BTX
    enum EDGE {LEFT, TOP, RIGHT, BOTTOM};
    //ETX

  public:
    // Description:
    // Instantiate the class.
    static vtkOrthogonalRegionSliceRepresentation *New();

    // Description:
    // Standard methods for the class.
    vtkTypeMacro(vtkOrthogonalRegionSliceRepresentation,vtkWidgetRepresentation);
    void PrintSelf(ostream& os, vtkIndent indent);

    void reset();

    // Description:
    // Get the view type properties. In which plane it is been shown
    // and which slice (in case of planar views) is selected
    virtual void SetView(View2D *view);
    virtual void SetPlane(Plane plane);
    virtual void SetSlice(Nm pos);
    virtual void SetOrthogonalBounds(double bounds[6]);
    virtual void GetOrthogonalBounds(double bounds[6]);

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
    vtkOrthogonalRegionSliceRepresentation();
    ~vtkOrthogonalRegionSliceRepresentation();

    // Manage how the representation appears
    double LastEventPosition[3];

    // Counting Region Edge
    vtkSmartPointer<vtkActor>          EdgeActor[4];
    vtkSmartPointer<vtkPolyDataMapper> EdgeMapper[4];
    vtkSmartPointer<vtkPolyData>       EdgePolyData[4];
    vtkSmartPointer<vtkPoints>         Vertex;

    void HighlightEdge(vtkSmartPointer<vtkActor> actor);
    void Highlight();

    // Do the picking
    vtkSmartPointer<vtkCellPicker> EdgePicker;
    vtkSmartPointer<vtkCellPicker> LastPicker;
    vtkSmartPointer<vtkActor>      CurrentEdge;

    // Properties used to control the appearance of selected objects and
    // the manipulator in general.
    vtkSmartPointer<vtkProperty> EdgeProperty;
    vtkSmartPointer<vtkProperty> SelectedEdgeProperty;
    vtkSmartPointer<vtkProperty> InvisibleProperty;

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
    vtkOrthogonalRegionSliceRepresentation(const vtkOrthogonalRegionSliceRepresentation&);  //Not implemented
    void operator=(const vtkOrthogonalRegionSliceRepresentation&);  //Not implemented

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
    View2D *m_view;
  };

} // namespace ESPINA

#endif
