/*
 <one line to give the program's name and a brief idea of what it does.>
 Copyright (C) 2012  Jorge Peña Pastor <jpena@cesvima.upm.es>

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 , CreateCenter, SetRadius*/

#ifndef VTKTUBULARREPRESENTATION_H
#define VTKTUBULARREPRESENTATION_H

// VTK
#include <vtkWidgetRepresentation.h>
#include <vtkUnstructuredGrid.h>
#include <vtkSmartPointer.h>
#include <vtkPlane.h>

// EspINA
#include <Core/EspinaTypes.h>
#include <Core/Filters/TubularSegmentationFilter.h>
#include "vtkTubularWidget.h"

class vtkSphereSource;
class vtkActor;
class vtkBox;
class vtkCellPicker;
class vtkPoints;
class vtkPolyData;
class vtkPolyDataMapper;
class vtkProperty;
class vtkRegularPolygonSource;

namespace EspINA
{
  class VTK_WIDGETS_EXPORT vtkTubularRepresentation
  : public vtkWidgetRepresentation
  {
    public:
      // Description:
      // Instantiate the class.
      static vtkTubularRepresentation *New();

      // Description:
      // Standard methods for the class.
      vtkTypeMacro(vtkTubularRepresentation,vtkWidgetRepresentation)
      ;
      void PrintSelf(ostream& os, vtkIndent indent);

      void reset();

      // Description:
      virtual void SetPlane(PlaneType plane);

      // Description:
      // These are methods that satisfy vtkWidgetRepresentation's API.
      virtual void PlaceWidget(double bounds[6]);
      virtual void BuildRepresentation();
      virtual int ComputeInteractionState(int X, int Y, int modify = 0);
      virtual void StartWidgetInteraction(double e[2]);
      virtual void WidgetInteraction(double e[2]);
      virtual double *GetBounds();

      // Description:
      // Methods supporting, and required by, the rendering process.
      virtual void ReleaseGraphicsResources(vtkWindow*);
      virtual int RenderOpaqueGeometry(vtkViewport*);
      virtual int RenderTranslucentPolygonalGeometry(vtkViewport*);
      virtual int HasTranslucentPolygonalGeometry();

//BTX - used to manage the state of the widget
      static const int MAX_NODES = 100;
      enum
      {
        CreatingNode = 0, MovingNode, ChangingRadius
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

      // get/set whole representation as a node list with world coordinates, each
      // widget should modify its actors according to its plane.
      TubularSegmentationFilter::NodeList GetNodeList(void);
      void SetNodeList(TubularSegmentationFilter::NodeList);

      // set the slice the widget is going to work and modify the representation
      // accordingly
      void SetSlice(Nm slice);

      // set/get boolean value indicating if the extremes of the segmentation are
      // round or not
      bool getRoundExtremes();
      void setRoundExtremes(bool);

    protected:
      vtkTubularRepresentation();
      ~vtkTubularRepresentation();

      // Manage how the representation appears
      double LastEventPosition[3];

      // Nodes
      vtkSmartPointer<vtkSphereSource> NodeSource[MAX_NODES];
      vtkSmartPointer<vtkPolyDataMapper> NodeMapper[MAX_NODES];
      vtkSmartPointer<vtkActor> NodeActor[MAX_NODES];
      vtkSmartPointer<vtkPolyDataMapper> NodeSegmentMapper[MAX_NODES];
      vtkSmartPointer<vtkActor> NodeSegment[MAX_NODES];
      vtkSmartPointer<vtkPolyDataMapper> RadiusMapper[MAX_NODES];
      vtkSmartPointer<vtkActor> RadiusActor[MAX_NODES];
      bool NodeIsVisible[MAX_NODES];
      bool SegmentIsVisible[MAX_NODES];

      void HighlightNode(vtkSmartPointer<vtkActor> actor);

      // Do the picking
      vtkSmartPointer<vtkCellPicker> widgetPicker;
      vtkSmartPointer<vtkActor> CurrentHandle;
      int currentNode;

      // Support GetBounds() method
      vtkSmartPointer<vtkBox> BoundingBox;

      // Properties used to control the appearance of selected objects and
      // the manipulator in general.
      vtkSmartPointer<vtkProperty> EdgeProperty;
      vtkSmartPointer<vtkProperty> NodeProperty;
      vtkSmartPointer<vtkProperty> SelectedNodeProperty;
      vtkSmartPointer<vtkProperty> LastNodeProperty;
      vtkSmartPointer<vtkProperty> DottedNodeProperty;

    private:
      vtkTubularRepresentation(const vtkTubularRepresentation&);  //Not implemented
      void operator=(const vtkTubularRepresentation&);  //Not implemented

      // the plane the widget is working
      PlaneType Plane;
      vtkSmartPointer<vtkPlane> cutterPlane;

      // actual working slice, needed to correctly updating coordinates
      Nm currentSlice;

      // internal list maintained by the representation
      TubularSegmentationFilter::NodeList nodes;

      // update node sections representation
      void updateSections(int node);

      bool m_roundExtremes;

      virtual void CreateDefaultProperties();

      // Directions are relative to the plane selected and corrected
      // when added to the node list
      void CreateNode(double *p);
      void MoveNode(double *p);
      void ChangeRadius(double *p);
  };
}
#endif // VTKTUBULARREPRESENTATION_H
