#include "vtkTubularRepresentation.h"

#include <vtkActor.h>
#include <vtkAssemblyPath.h>
#include <vtkBox.h>
#include <vtkCamera.h>
#include <vtkCellArray.h>
#include <vtkCellPicker.h>
#include <vtkDoubleArray.h>
#include <vtkInteractorObserver.h>
#include <vtkLine.h>
#include <vtkMath.h>
#include <vtkObjectFactory.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRegularPolygonSource.h>
#include <vtkSphereSource.h>
#include <vtkRenderer.h>
#include <vtkWindow.h>
#include <vtkPointData.h>
#include <vtkPropCollection.h>
#include <QVector3D>
#include <vtkCutter.h>
#include <vtkPlane.h>
#include "vtkTubeSource.h"
#include <vtkTransform.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkRenderWindow.h>

namespace EspINA
{
  vtkStandardNewMacro(vtkTubularRepresentation);

  //----------------------------------------------------------------------------
  vtkTubularRepresentation::vtkTubularRepresentation()
  : currentNode(-1)
  , Plane(AXIAL)
  , currentSlice(0)
  , m_roundExtremes(false)
  {
    // The initial state
    this->InteractionState = vtkTubularRepresentation::CreatingNode;

    this->cutterPlane = vtkSmartPointer<vtkPlane>::New();
    this->cutterPlane->SetOrigin(0, 0, 0);
    this->cutterPlane->SetNormal(0, 0, 1);
    this->cutterPlane->Modified();

    this->widgetPicker = vtkSmartPointer<vtkCellPicker>::New();
    this->widgetPicker->SetTolerance(0.01);
    this->widgetPicker->PickFromListOn();

    // Define the point coordinates
    double bounds[6] = { -50, 50, -50, 50, -50, 50 };
    this->BoundingBox = vtkSmartPointer<vtkBox>::New();
    this->BoundingBox->SetBounds(bounds);
    this->PlaceWidget(bounds);

    this->CurrentHandle = NULL;

    for (int i = 0; i < MAX_NODES; i++)
    {
      this->NodeIsVisible[i] = false;
      this->SegmentIsVisible[i] = false;
    }

    this->CreateDefaultProperties();
    this->BuildRepresentation();
  }

  //----------------------------------------------------------------------------
  vtkTubularRepresentation::~vtkTubularRepresentation()
  {
    // remove all actors from renderer before leaving
    for (int i = 0; i < this->nodes.size(); i++)
    {
      if (this->NodeIsVisible[i])
      {
        this->Renderer->RemoveActor(this->NodeActor[i]);
        this->Renderer->RemoveActor(this->RadiusActor[i]);
        this->widgetPicker->DeletePickList(this->NodeActor[i]);
        this->widgetPicker->DeletePickList(this->RadiusActor[i]);
      }

      if (this->SegmentIsVisible[i])
        this->Renderer->RemoveActor(this->NodeSegment[i]);
    }

    this->Renderer->GetRenderWindow()->Render();
  }

  //----------------------------------------------------------------------
  void vtkTubularRepresentation::reset()
  {
    if (this->nodes.empty())
      return;

    for (int i = 0; i < nodes.size(); i++)
    {
      if (this->NodeIsVisible[i])
      {
        this->NodeIsVisible[i] = false;

        this->Renderer->RemoveActor(this->NodeActor[i]);
        this->Renderer->RemoveActor(this->RadiusActor[i]);
        this->widgetPicker->DeletePickList(this->NodeActor[i]);
        this->widgetPicker->DeletePickList(this->RadiusActor[i]);

        this->RadiusMapper[i] = NULL;
        this->RadiusActor[i] = NULL;
      }

      this->NodeActor[i] = NULL;
      this->NodeMapper[i] = NULL;
      this->NodeSource[i] = NULL;

      if (this->SegmentIsVisible[i])
      {
        this->Renderer->RemoveActor(this->NodeSegment[i]);
        this->SegmentIsVisible[i] = false;
        this->NodeSegment[i] = NULL;
        this->NodeMapper[i] = NULL;
      }
    }

    nodes.clear();
  }

  //----------------------------------------------------------------------
  void vtkTubularRepresentation::StartWidgetInteraction(double e[2])
  {
    // Store the start position
    this->StartEventPosition[0] = e[0];
    this->StartEventPosition[1] = e[1];
    this->StartEventPosition[2] = 0;

    // Store the start position
    this->LastEventPosition[0] = e[0];
    this->LastEventPosition[1] = e[1];
    this->LastEventPosition[2] = 0;

    this->ComputeInteractionState(static_cast<int>(e[0]), static_cast<int>(e[1]), 0);
  }

  //----------------------------------------------------------------------
  void vtkTubularRepresentation::WidgetInteraction(double e[2])
  {
    // Convert events to appropriate coordinate systems
    vtkCamera *camera = this->Renderer->GetActiveCamera();
    if (!camera)
      return;

    double focalPoint[3], pickPoint[3], prevPickPoint[3];
    double z, vpn[3];
    camera->GetViewPlaneNormal(vpn);

    // Compute the two points defining the motion vector
    double pos[3];
    this->widgetPicker->GetPickPosition(pos);

    vtkInteractorObserver::ComputeWorldToDisplay(this->Renderer, pos[0], pos[1], pos[2], focalPoint);
    z = focalPoint[2];
    vtkInteractorObserver::ComputeDisplayToWorld(this->Renderer, this->LastEventPosition[0], this->LastEventPosition[1],
        z, prevPickPoint);
    vtkInteractorObserver::ComputeDisplayToWorld(this->Renderer, e[0], e[1], z, pickPoint);

    // Process the motion
    if (this->InteractionState == vtkTubularRepresentation::CreatingNode)
      this->CreateNode(pickPoint);
    else
      if (this->InteractionState == vtkTubularRepresentation::MovingNode)
        this->MoveNode(pickPoint);
      else
        if (this->InteractionState == vtkTubularRepresentation::ChangingRadius)
          this->ChangeRadius(pickPoint);

    // Store the start position
    this->LastEventPosition[0] = e[0];
    this->LastEventPosition[1] = e[1];
    this->LastEventPosition[2] = 0;
  }

  //----------------------------------------------------------------------------
  void vtkTubularRepresentation::CreateNode(double* p)
  {
    if (nodes.size() == MAX_NODES)
      return;

    const int R = 2; //Node's radius on creation
    double bounds[6];

    switch (this->Plane)
    {
      case AXIAL:
        this->nodes << QVector4D(p[0], p[1], currentSlice, R);
        bounds[0] = p[0] - R;
        bounds[1] = p[0] + R;
        bounds[2] = p[1] - R;
        bounds[3] = p[1] + R;
        bounds[4] = currentSlice - R;
        bounds[5] = currentSlice + R;
        p[2] = -0.1;
        break;
      case CORONAL:
        this->nodes << QVector4D(p[0], currentSlice, p[2], R);
        bounds[0] = p[0] - R;
        bounds[1] = p[0] + R;
        bounds[2] = currentSlice - R;
        bounds[3] = currentSlice + R;
        bounds[4] = p[2] - R;
        bounds[5] = p[2] + R;
        p[1] = +0.1;
        break;
      case SAGITTAL:
        this->nodes << QVector4D(currentSlice, p[1], p[2], R);
        bounds[0] = currentSlice - R;
        bounds[1] = currentSlice + R;
        bounds[2] = p[1] - R;
        bounds[3] = p[1] + R;
        bounds[4] = p[2] - R;
        bounds[5] = p[2] + R;
        p[0] = +0.1;
        break;
      default:
        Q_ASSERT(FALSE);
        break;
    }

    this->currentNode = this->nodes.size() - 1;

    if (0 == currentNode)
      this->BoundingBox->SetBounds(bounds);
    else
      this->BoundingBox->AddBounds(bounds);

    this->NodeSource[currentNode] = vtkSmartPointer<vtkSphereSource>::New();
    this->NodeSource[currentNode]->SetRadius(2);
    this->NodeSource[currentNode]->SetCenter(p);
    this->NodeMapper[currentNode] = vtkSmartPointer<vtkPolyDataMapper>::New();
    this->NodeMapper[currentNode]->SetInputConnection(this->NodeSource[currentNode]->GetOutputPort());
    this->NodeMapper[currentNode]->SetResolveCoincidentTopologyToPolygonOffset();
    this->NodeActor[currentNode] = vtkSmartPointer<vtkActor>::New();
    this->NodeActor[currentNode]->SetMapper(NULL);
    this->NodeActor[currentNode]->SetProperty(this->SelectedNodeProperty);
    this->NodeIsVisible[currentNode] = false;
    this->SegmentIsVisible[currentNode] = false;

    if (!m_roundExtremes)
      this->SelectedNodeProperty->SetLineStipplePattern(0xCCCC);

    updateSections(currentNode);
  }

  //----------------------------------------------------------------------------
  void vtkTubularRepresentation::MoveNode(double* p)
  {
    switch (this->Plane)
    {
      case AXIAL:
        nodes[currentNode].setX(p[0]);
        nodes[currentNode].setY(p[1]);
        p[2] = -0.1;
        break;
      case CORONAL:
        nodes[currentNode].setX(p[0]);
        nodes[currentNode].setZ(p[2]);
        p[1] = +0.1;
        break;
      case SAGITTAL:
        nodes[currentNode].setY(p[1]);
        nodes[currentNode].setZ(p[2]);
        p[0] = +0.1;
        break;
      default:
        Q_ASSERT(FALSE);
        break;
    }

    this->NodeSource[currentNode]->SetCenter(p);

    // we need to rebuild bounds
    TubularSegmentationFilter::NodeList::Iterator it;
    for (it = nodes.begin(); it != nodes.end(); it++)
    {
      double R = (*it).w();
      double center[3] = { (*it).x(), (*it).y(), (*it).z() };
      double bounds[6] = { center[0] - R, center[0] + R, center[1] - R, center[1] + R, center[2] - R, center[2] + R };
      if (it == nodes.begin())
        this->BoundingBox->SetBounds(bounds);
      else
        this->BoundingBox->AddBounds(bounds);
    }

    updateSections(currentNode);

    if (currentNode != (this->nodes.size() - 1))
      updateSections(currentNode + 1);
  }

  //----------------------------------------------------------------------------
  void vtkTubularRepresentation::ChangeRadius(double* p)
  {
    switch (this->Plane)
    {
      case AXIAL:
        p[2] = this->currentSlice;
        break;
      case CORONAL:
        p[1] = this->currentSlice;
        break;
      case SAGITTAL:
        p[0] = this->currentSlice;
        break;
      default:
        Q_ASSERT(FALSE);
        break;
    }

    double c[3] = { this->nodes[currentNode].x(), this->nodes[currentNode].y(), this->nodes[currentNode].z() };
    double r = sqrt(vtkMath::Distance2BetweenPoints(c, p));
    this->nodes[currentNode].setW(r);

    double bounds[6] = { c[0] - r, c[0] + r, c[1] - r, c[1] + r, c[2] + r, c[2] + r };
    if (nodes.size() == 1)
      this->BoundingBox->SetBounds(bounds);
    else
      this->BoundingBox->AddBounds(bounds);

    updateSections(currentNode);

    if (currentNode != (this->nodes.size() - 1))
      updateSections(currentNode + 1);
  }

  //----------------------------------------------------------------------------
  void vtkTubularRepresentation::CreateDefaultProperties()
  {
    this->EdgeProperty = vtkSmartPointer<vtkProperty>::New();
    this->EdgeProperty->SetAmbient(1.0);
    this->EdgeProperty->SetDiffuse(0.0);
    this->EdgeProperty->SetSpecular(0.0);
    this->EdgeProperty->SetLineWidth(2.0);
    this->EdgeProperty->SetColor(0, 1, 0);

    this->NodeProperty = vtkSmartPointer<vtkProperty>::New();
    this->NodeProperty->SetAmbient(1.0);
    this->NodeProperty->SetDiffuse(0.0);
    this->NodeProperty->SetSpecular(0.0);
    this->NodeProperty->SetLineWidth(2.0);
    this->NodeProperty->SetColor(1, 0, 1);

    this->SelectedNodeProperty = vtkSmartPointer<vtkProperty>::New();
    this->SelectedNodeProperty->SetAmbient(1.0);
    this->SelectedNodeProperty->SetDiffuse(0.0);
    this->SelectedNodeProperty->SetSpecular(0.0);
    this->SelectedNodeProperty->SetLineWidth(2.0);
    this->SelectedNodeProperty->SetColor(1, 1, 1);

    this->LastNodeProperty = vtkSmartPointer<vtkProperty>::New();
    this->LastNodeProperty->SetAmbient(1.0);
    this->LastNodeProperty->SetDiffuse(0.0);
    this->LastNodeProperty->SetSpecular(0.0);
    this->LastNodeProperty->SetLineWidth(3.0);
    this->LastNodeProperty->SetColor(1, 0, 0);
    this->LastNodeProperty->SetRepresentationToSurface();

    if (!m_roundExtremes)
      this->LastNodeProperty->SetLineStipplePattern(0xCCCC);

    this->DottedNodeProperty = vtkSmartPointer<vtkProperty>::New();
    this->DottedNodeProperty->SetLineStipplePattern(0xCCCC);
    this->DottedNodeProperty->SetAmbient(1.0);
    this->DottedNodeProperty->SetDiffuse(0.0);
    this->DottedNodeProperty->SetSpecular(0.0);
    this->DottedNodeProperty->SetLineWidth(2.0);
    this->DottedNodeProperty->SetColor(1, 0, 1);
  }

  //----------------------------------------------------------------------------
  void vtkTubularRepresentation::SetPlane(PlaneType plane)
  {
    Plane = plane;
  }

  //----------------------------------------------------------------------------
  void vtkTubularRepresentation::PlaceWidget(double bds[6])
  {
    int i;
    double bounds[6], center[3];

    this->AdjustBounds(bds, bounds, center);

    for (i = 0; i < 6; i++)
      this->InitialBounds[i] = bounds[i];

    this->InitialLength = sqrt(
        (bounds[1] - bounds[0]) * (bounds[1] - bounds[0]) + (bounds[3] - bounds[2]) * (bounds[3] - bounds[2])
            + (bounds[5] - bounds[4]) * (bounds[5] - bounds[4]));

    this->ValidPick = 1; //since we have set up widget
  }

  //----------------------------------------------------------------------------
  int vtkTubularRepresentation::ComputeInteractionState(int X, int Y, int modify)
  {
    // Okay, we can process this. Try to pick handles first;
    // if no handles picked, then pick the bounding box.
    if (!this->Renderer || !this->Renderer->IsInViewport(X, Y))
    {
      this->InteractionState = vtkTubularRepresentation::CreatingNode;
      return this->InteractionState;
    }

    vtkAssemblyPath *path;
    this->CurrentHandle = NULL;
    this->widgetPicker->Pick(X, Y, 0.0, this->Renderer);
    path = this->widgetPicker->GetPath();
    if (path != NULL)
    {
      this->ValidPick = 1;

      vtkProp *prop = path->GetFirstNode()->GetViewProp();
      this->CurrentHandle = reinterpret_cast<vtkActor *>(prop);
      for (int i = 0; i < nodes.size(); i++)
      {
        if (!this->NodeIsVisible[i])
          continue;

        if (this->NodeActor[i] == this->CurrentHandle)
        {
          this->currentNode = i;
          this->InteractionState = vtkTubularRepresentation::MovingNode;
          return this->InteractionState;
        }
        else
        {
          if (this->RadiusActor[i] == this->CurrentHandle)
          {
            this->currentNode = i;
            this->InteractionState = vtkTubularRepresentation::ChangingRadius;
            return this->InteractionState;
          }
        }
      }
    }

    this->InteractionState = vtkTubularRepresentation::CreatingNode;
    this->currentNode = -1;
    return this->InteractionState;
  }

  //----------------------------------------------------------------------
  void vtkTubularRepresentation::SetInteractionState(int state)
  {
    // Clamp to allowable values
    state = (state < vtkTubularRepresentation::CreatingNode ? vtkTubularRepresentation::CreatingNode : state);

    // Depending on state, highlight appropriate parts of representation
    this->InteractionState = state;
    switch (state)
    {
      case vtkTubularRepresentation::CreatingNode:
      case vtkTubularRepresentation::ChangingRadius:
      case vtkTubularRepresentation::MovingNode:
        this->HighlightNode(this->CurrentHandle);
        break;
      default:
        this->HighlightNode(NULL);
        break;
    }
  }

  //----------------------------------------------------------------------
  double *vtkTubularRepresentation::GetBounds()
  {
    this->BuildRepresentation();
    return this->BoundingBox->GetBounds();
  }

  //----------------------------------------------------------------------------
  void vtkTubularRepresentation::BuildRepresentation()
  {
    // Rebuild only if necessary
    if (this->GetMTime() > this->BuildTime
        || (this->Renderer && this->Renderer->GetVTKWindow()
            && (this->Renderer->GetVTKWindow()->GetMTime() > this->BuildTime
                || this->Renderer->GetActiveCamera()->GetMTime() > this->BuildTime)))
    {
      this->BuildTime.Modified();
    }
  }

  //----------------------------------------------------------------------------
  void vtkTubularRepresentation::ReleaseGraphicsResources(vtkWindow *w)
  {
    for (int i = 0; i < nodes.size(); i++)
    {
      this->NodeActor[i]->ReleaseGraphicsResources(w);
      if (this->NodeIsVisible[i])
        this->RadiusActor[i]->ReleaseGraphicsResources(w);
      if (this->SegmentIsVisible[i])
        this->NodeSegment[i]->ReleaseGraphicsResources(w);
    }
  }

  //----------------------------------------------------------------------------
  int vtkTubularRepresentation::RenderOpaqueGeometry(vtkViewport *v)
  {
    int count = 0;
    this->BuildRepresentation();

    for (int i = 0; i < nodes.size(); i++)
    {
      count += this->NodeActor[i]->RenderOpaqueGeometry(v);
      if (this->NodeIsVisible[i])
        count += this->RadiusActor[i]->RenderOpaqueGeometry(v);
      if (this->SegmentIsVisible[i])
        count += this->NodeSegment[i]->RenderOpaqueGeometry(v);
    }

    return count;
  }

  //----------------------------------------------------------------------------
  int vtkTubularRepresentation::RenderTranslucentPolygonalGeometry(vtkViewport *v)
  {
    int count = 0;
    this->BuildRepresentation();

    for (int i = 0; i < nodes.size(); i++)
    {
      count += this->NodeActor[i]->RenderTranslucentPolygonalGeometry(v);
      if (this->NodeIsVisible[i])
        count += this->RadiusActor[i]->RenderTranslucentPolygonalGeometry(v);
      if (this->SegmentIsVisible[i])
        count += this->NodeSegment[i]->RenderTranslucentPolygonalGeometry(v);
    }

    return count;
  }

  //----------------------------------------------------------------------------
  int vtkTubularRepresentation::HasTranslucentPolygonalGeometry()
  {
    int result = 0;
    this->BuildRepresentation();

    for (int i = 0; i < nodes.size(); i++)
    {
      result |= this->NodeActor[i]->HasTranslucentPolygonalGeometry();
      if (this->NodeIsVisible[i])
        result |= this->RadiusActor[i]->HasTranslucentPolygonalGeometry();
      if (this->SegmentIsVisible[i])
        result |= this->NodeSegment[i]->HasTranslucentPolygonalGeometry();
    }

    return result;
  }

  //----------------------------------------------------------------------------
  void vtkTubularRepresentation::HighlightNode(vtkSmartPointer<vtkActor> actor)
  {
    for (int i = 0; i < nodes.size(); i++)
    {
      if (!this->NodeIsVisible[i])
        continue;

      if (this->NodeActor[i] == actor)
      {
        this->NodeActor[i]->SetProperty(this->SelectedNodeProperty);
        this->RadiusActor[i]->SetProperty(this->SelectedNodeProperty);

        if (((0 == i) || (nodes.size() - 1 == i)) && !m_roundExtremes)
        {
          this->SelectedNodeProperty->SetLineStipplePattern(0xCCCC);
          this->SelectedNodeProperty->Modified();
        }
      }
      else
      {
        if (this->nodes.size() - 1 == i)
        {
          this->NodeActor[i]->SetProperty(this->LastNodeProperty);
          this->RadiusActor[i]->SetProperty(this->LastNodeProperty);
        }
        else
          if ((0 == i) && !m_roundExtremes)
          {
            this->NodeActor[i]->SetProperty(this->DottedNodeProperty);
            this->RadiusActor[i]->SetProperty(this->DottedNodeProperty);
          }
          else
          {
            this->NodeActor[i]->SetProperty(this->NodeProperty);
            this->RadiusActor[i]->SetProperty(this->NodeProperty);
          }
      }
    }
  }

  //----------------------------------------------------------------------------
  void vtkTubularRepresentation::PrintSelf(ostream& os, vtkIndent indent)
  {
    this->Superclass::PrintSelf(os, indent);

    double *bounds = this->InitialBounds;
    os << indent << "Initial Bounds: " << "(" << bounds[0] << "," << bounds[1] << ") " << "(" << bounds[2] << ","
        << bounds[3] << ") " << "(" << bounds[4] << "," << bounds[5] << ")\n";

    if (this->SelectedNodeProperty)
    {
      os << indent << "Selected Outline Property: " << this->SelectedNodeProperty << "\n";
    }
    else
    {
      os << indent << "Selected Outline Property: (none)\n";
    }

  }

  //----------------------------------------------------------------------------
  TubularSegmentationFilter::NodeList vtkTubularRepresentation::GetNodeList()
  {
    TubularSegmentationFilter::NodeList list = this->nodes;

    if (vtkTubularRepresentation::ChangingRadius == this->InteractionState)
      list.removeLast();

    return list;
  }

  //----------------------------------------------------------------------------
  void vtkTubularRepresentation::SetNodeList(TubularSegmentationFilter::NodeList list)
  {
    if (list == this->nodes)
      return;

    this->reset();
    this->nodes = list;

    TubularSegmentationFilter::NodeList::Iterator it = this->nodes.begin();
    for (int i = 0; i < this->nodes.size(); i++, it++)
    {
      double data[3] = { 0,0,0 };

      switch (this->Plane)
      {
        case AXIAL:
          data[0] = (*it).x();
          data[1] = (*it).y();
          data[2] = -0.1;
          break;
        case CORONAL:
          data[0] = (*it).x();
          data[1] = +0.1;
          data[2] = (*it).z();
          break;
        case SAGITTAL:
          data[0] = +0.1;
          data[1] = (*it).y();
          data[2] = (*it).z();
          break;
        default:
          Q_ASSERT(FALSE);
          break;
      }

      this->NodeSource[i] = vtkSmartPointer<vtkSphereSource>::New();
      this->NodeSource[i]->SetRadius(2);
      this->NodeSource[i]->SetCenter(data[0], data[1], data[2]);
      this->NodeSource[i]->Update();
      this->NodeMapper[i] = vtkSmartPointer<vtkPolyDataMapper>::New();
      this->NodeMapper[i]->SetInputConnection(this->NodeSource[i]->GetOutputPort());
      this->NodeMapper[i]->SetResolveCoincidentTopologyToPolygonOffset();
      this->NodeActor[i] = vtkSmartPointer<vtkActor>::New();
      this->NodeActor[i]->SetMapper(NULL);
      this->NodeActor[i]->SetProperty(this->LastNodeProperty);
      if (i > 0)
      {
        if (!m_roundExtremes && ((i - 1) == 0))
          this->NodeActor[0]->SetProperty(this->DottedNodeProperty);
        else
          this->NodeActor[i - 1]->SetProperty(this->NodeProperty);
      }

      this->NodeIsVisible[i] = false;
      this->SegmentIsVisible[i] = false;

      double bounds[6] = { (*it).x() - (*it).w(), (*it).x() + (*it).w(), (*it).y() - (*it).w(), (*it).y() + (*it).w(),
          (*it).z() - (*it).w(), (*it).z() + (*it).w() };

      if (0 == i)
        this->BoundingBox->SetBounds(bounds);
      else
        this->BoundingBox->AddBounds(bounds);

      updateSections(i);
    }
    this->InteractionState = vtkTubularRepresentation::CreatingNode;
    this->currentNode = -1;
  }

  //----------------------------------------------------------------------------
  void vtkTubularRepresentation::SetSlice(Nm slice)
  {
    this->currentSlice = slice;

    switch (this->Plane)
    {
      case AXIAL:
        this->cutterPlane->SetOrigin(0, 0, slice);
        this->cutterPlane->SetNormal(0, 0, 1);
        break;
      case CORONAL:
        this->cutterPlane->SetOrigin(0, slice, 0);
        this->cutterPlane->SetNormal(0, 1, 0);
        break;
      case SAGITTAL:
        this->cutterPlane->SetOrigin(slice, 0, 0);
        this->cutterPlane->SetNormal(1, 0, 0);
        break;
      default:
        Q_ASSERT(FALSE);
        break;
    }
    this->cutterPlane->Modified();

    for (int i = 0; i < this->nodes.size(); i++)
      updateSections(i);
  }

  //----------------------------------------------------------------------------
  void vtkTubularRepresentation::updateSections(int node)
  {
    double distance = 0;
    double nodeZ = 0;
    double previousNodeZ = 0;

    switch (this->Plane)
    {
      case AXIAL:
        distance = abs(this->currentSlice - this->nodes[node].z());
        nodeZ = this->nodes[node].z();
        previousNodeZ = (node > 0) ? this->nodes[node - 1].z() : 0;
        break;
      case CORONAL:
        distance = abs(this->currentSlice - this->nodes[node].y());
        nodeZ = this->nodes[node].y();
        previousNodeZ = (node > 0) ? this->nodes[node - 1].y() : 0;
        break;
      case SAGITTAL:
        distance = abs(this->currentSlice - this->nodes[node].x());
        nodeZ = this->nodes[node].x();
        previousNodeZ = (node > 0) ? this->nodes[node - 1].x() : 0;
        break;
      default:
        Q_ASSERT(FALSE);
        break;
    }

    if (true == this->NodeIsVisible[node])
    {
      this->NodeIsVisible[node] = false;
      this->widgetPicker->DeletePickList(this->NodeActor[node]);
      this->widgetPicker->DeletePickList(this->RadiusActor[node]);
      this->Renderer->RemoveActor(this->NodeActor[node]);
      this->NodeActor[node]->SetMapper(NULL); // needed to hide the actor ¿¿??
      this->Renderer->RemoveActor(this->RadiusActor[node]);
      this->NodeActor[node]->SetPickable(false);
    }

    // modify spheres visibility and radius
    if (distance <= this->nodes[node].w())
    {
      this->NodeIsVisible[node] = true;
      this->NodeActor[node]->SetMapper(this->NodeMapper[node]); // reconnect the mapper
      this->NodeActor[node]->SetPickable(true);
      this->Renderer->AddActor(this->NodeActor[node]);
      this->widgetPicker->AddPickList(this->NodeActor[node]);

      vtkSphereSource *sphere = vtkSphereSource::New();
      sphere->SetThetaResolution(50);
      sphere->SetPhiResolution(50);
      sphere->SetLatLongTessellation(true);
      sphere->SetCenter(this->nodes[node].x(), this->nodes[node].y(), this->nodes[node].z());
      sphere->SetRadius(this->nodes[node].w());
      sphere->Modified();

      vtkCutter *cutter = vtkCutter::New();
      cutter->SetNumberOfContours(1);
      cutter->SetCutFunction(this->cutterPlane);
      cutter->GenerateCutScalarsOff();
      cutter->SetInputConnection(sphere->GetOutputPort());
      cutter->Update();

      this->RadiusMapper[node] = vtkSmartPointer<vtkPolyDataMapper>::New();
      this->RadiusMapper[node]->ScalarVisibilityOff();
      this->RadiusMapper[node]->SetInputConnection(cutter->GetOutputPort());
      this->RadiusMapper[node]->SetResolveCoincidentTopologyToPolygonOffset();
      this->RadiusMapper[node]->Update();

      this->RadiusActor[node] = vtkSmartPointer<vtkActor>::New();
      this->RadiusActor[node]->SetMapper(this->RadiusMapper[node]);

      // set the property of the node actors
      if (node != this->currentNode)
      {
        if (node == this->nodes.size() - 1)
        {
          this->NodeActor[node]->SetProperty(this->LastNodeProperty);
          this->RadiusActor[node]->SetProperty(this->LastNodeProperty);
        }
        else
        {
          if ((0 == node) && !m_roundExtremes)
          {
            this->NodeActor[node]->SetProperty(this->DottedNodeProperty);
            this->RadiusActor[node]->SetProperty(this->DottedNodeProperty);
          }
          else
          {
            this->NodeActor[node]->SetProperty(this->NodeProperty);
            this->RadiusActor[node]->SetProperty(this->NodeProperty);
          }
        }
      }
      else
      {
        if (((0 == node) || (this->nodes.size() - 1 == node)) && !m_roundExtremes)
          this->SelectedNodeProperty->SetLineStipplePattern(0xCCCC);
        else
          this->SelectedNodeProperty->SetLineStipplePattern(0xFFFF);

        this->NodeActor[node]->SetProperty(this->SelectedNodeProperty);
        this->RadiusActor[node]->SetProperty(this->SelectedNodeProperty);
      }

      switch (this->Plane)
      {
        case AXIAL:
          this->RadiusActor[node]->SetPosition(0, 0, -this->currentSlice - 0.1);
          break;
        case CORONAL:
          this->RadiusActor[node]->SetPosition(0, -this->currentSlice + 0.1, 0);
          break;
        case SAGITTAL:
          this->RadiusActor[node]->SetPosition(-this->currentSlice + 0.1, 0, 0);
          break;
        default:
          Q_ASSERT(FALSE);
          break;
      }

      this->Renderer->AddActor(this->RadiusActor[node]);
      this->widgetPicker->AddPickList(this->RadiusActor[node]);

      sphere->Delete();
      cutter->Delete();
    }

    // modify edges
    if (this->SegmentIsVisible[node])
    {
      this->SegmentIsVisible[node] = false;
      this->Renderer->RemoveActor(this->NodeSegment[node]);
    }

    if (0 == node)
      return;

    Nm min = (nodeZ < previousNodeZ) ? nodeZ : previousNodeZ;
    Nm max = (nodeZ > previousNodeZ) ? nodeZ : previousNodeZ;

    if (((max >= this->currentSlice) && (min <= this->currentSlice))
        || (this->currentSlice >= nodeZ && this->currentSlice <= previousNodeZ) || this->NodeIsVisible[node]
        || this->NodeIsVisible[node - 1])
    {
      QVector3D origin1(this->nodes[node - 1]);
      QVector3D origin2(this->nodes[node]);
      QVector3D direction = origin2 - origin1;

      if (direction.length() < 0.01)
        return;

      direction.normalize();
      this->SegmentIsVisible[node] = true;

      double up[3] = { 0, 1, 0 };
      double dir[3] = { direction.x(), direction.y(), direction.z() };
      double axis[3];
      vtkMath::Cross(up, dir, axis);
      double costheta = vtkMath::Dot(dir, up);
      double sintheta = sqrt(1 - costheta * costheta);
      double t = 1 - costheta;

      QVector3D qaxis(axis[0], axis[1], axis[2]);
      qaxis.normalize();

      vtkTubeSource *tube = vtkTubeSource::New();
      tube->SetResolution(200);
      tube->SetBottomRadius(this->nodes[node - 1].w());
      tube->SetBottomCenter(this->nodes[node - 1].x(), this->nodes[node - 1].y(), this->nodes[node - 1].z());
      tube->SetTopRadius(this->nodes[node].w());
      tube->SetTopCenter(this->nodes[node].x(), this->nodes[node].y(), this->nodes[node].z());

      // build the rotation matrix
      vtkMatrix4x4 *matrix = vtkMatrix4x4::New();
      matrix->SetElement(0, 0, t * qaxis.x() * qaxis.x() + costheta);
      matrix->SetElement(0, 1, t * qaxis.x() * qaxis.y() - sintheta * qaxis.z());
      matrix->SetElement(0, 2, t * qaxis.x() * qaxis.z() + sintheta * qaxis.y());
      matrix->SetElement(0, 3, 0);

      matrix->SetElement(1, 0, t * qaxis.x() * qaxis.y() + sintheta * qaxis.z());
      matrix->SetElement(1, 1, t * qaxis.y() * qaxis.y() + costheta);
      matrix->SetElement(1, 2, t * qaxis.y() * qaxis.z() - sintheta * qaxis.x());
      matrix->SetElement(1, 3, 0);

      matrix->SetElement(2, 0, t * qaxis.x() * qaxis.z() - sintheta * qaxis.y());
      matrix->SetElement(2, 1, t * qaxis.y() * qaxis.z() + sintheta * qaxis.x());
      matrix->SetElement(2, 2, t * qaxis.z() * qaxis.z() + costheta);
      matrix->SetElement(2, 3, 0);

      matrix->SetElement(3, 0, 0);
      matrix->SetElement(3, 1, 0);
      matrix->SetElement(3, 2, 0);
      matrix->SetElement(3, 3, 1);

      vtkTransform *trans = vtkTransform::New();
      trans->PostMultiply();
      trans->SetMatrix(matrix);
      QVector3D point = origin1 + (0.5 * (origin2 - origin1));
      trans->Translate(point.x(), point.y(), point.z());

      vtkTransformPolyDataFilter *pdtrans = vtkTransformPolyDataFilter::New();
      pdtrans->SetInputConnection(tube->GetOutputPort());
      pdtrans->SetTransform(trans);

      vtkCutter *cutter = vtkCutter::New();
      cutter->SetNumberOfContours(1);
      cutter->SetCutFunction(this->cutterPlane);
      cutter->GenerateCutScalarsOff();
      cutter->SetInputConnection(pdtrans->GetOutputPort());
      cutter->Update();

      this->NodeSegmentMapper[node] = vtkSmartPointer<vtkPolyDataMapper>::New();
      this->NodeSegmentMapper[node]->ScalarVisibilityOff();
      this->NodeSegmentMapper[node]->SetInputConnection(cutter->GetOutputPort());
      this->NodeSegmentMapper[node]->SetResolveCoincidentTopologyToPolygonOffset();
      this->NodeSegmentMapper[node]->Update();

      this->NodeSegment[node] = vtkSmartPointer<vtkActor>::New();
      this->NodeSegment[node]->SetMapper(this->NodeSegmentMapper[node]);
      this->NodeSegment[node]->SetProperty(this->EdgeProperty);

      switch (this->Plane)
      {
        case AXIAL:
          this->NodeSegment[node]->SetPosition(0, 0, -this->currentSlice - 0.1);
          break;
        case CORONAL:
          this->NodeSegment[node]->SetPosition(0, -this->currentSlice + 0.1, 0);
          break;
        case SAGITTAL:
          this->NodeSegment[node]->SetPosition(-this->currentSlice + 0.1, 0, 0);
          break;
        default:
          Q_ASSERT(FALSE);
          break;
      }

      this->Renderer->AddActor(this->NodeSegment[node]);

      tube->Delete();
      cutter->Delete();
    }
  }

  //----------------------------------------------------------------------------
  bool vtkTubularRepresentation::getRoundExtremes()
  {
    return this->m_roundExtremes;
  }

  //----------------------------------------------------------------------------
  void vtkTubularRepresentation::setRoundExtremes(bool value)
  {
    if (value)
      this->LastNodeProperty->SetLineStipplePattern(0xFFFF);
    else
      this->LastNodeProperty->SetLineStipplePattern(0xCCCC);

    this->m_roundExtremes = value;
  }
}
