/*

 Copyright (C) 2015 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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

#include "GUI/Selectors/Selector.h"
#include <GUI/ColorEngines/ColorEngine.h>
#include <GUI/Model/SegmentationAdapter.h>
#include <Core/Utils/Spatial.h>
#include <GUI/View/Widgets/Contour/vtkPlaneContourRepresentationGlyph.h>

// VTK
#include <vtkCleanPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkObjectFactory.h>
#include <vtkProperty.h>
#include <vtkAssemblyPath.h>
#include <vtkMath.h>
#include <vtkInteractorObserver.h>
#include <vtkLine.h>
#include <vtkCoordinate.h>
#include <vtkGlyph3D.h>
#include <vtkCursor2D.h>
#include <vtkPolyData.h>
#include <vtkPoints.h>
#include <vtkDoubleArray.h>
#include <vtkPointData.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkTransform.h>
#include <vtkCamera.h>
#include <vtkPoints.h>
#include <vtkCellArray.h>
#include <vtkFocalPlanePointPlacer.h>
#include <vtkBezierContourLineInterpolator.h>
#include <vtkOpenGL.h>
#include <vtkSphereSource.h>
#include <vtkIncrementalOctreePointLocator.h>
#include <vtkPolygon.h>

// Qt
#include <QtGlobal>

using namespace ESPINA;

vtkStandardNewMacro(vtkPlaneContourRepresentationGlyph);

//----------------------------------------------------------------------------
vtkPlaneContourRepresentationGlyph::vtkPlaneContourRepresentationGlyph()
{
  this->Property = nullptr;
  this->ActiveProperty = nullptr;
  this->LinesProperty = nullptr;
  this->m_polygon = nullptr;
  this->m_polygonFilter = nullptr;
  this->m_polygonMapper = nullptr;
  this->useContourPolygon = false;
  this->m_polygonColor = Qt::black;

  // Initialize state
  this->InteractionState = vtkPlaneContourRepresentation::Outside;

  this->Spacing[0] = 1.0;
  this->Spacing[1] = 1.0;
  this->HandleSize = 0.01;
  this->PointPlacer = vtkFocalPlanePointPlacer::New();
  this->LineInterpolator = vtkBezierContourLineInterpolator::New();

  // Represent the position of the cursor
  this->FocalPoint = vtkSmartPointer<vtkPoints>::New();
  this->FocalPoint->SetNumberOfPoints(1);
  this->FocalPoint->SetPoint(0, 0.0, 0.0, 0.0);

  vtkDoubleArray *normals = vtkDoubleArray::New();
  normals->SetNumberOfComponents(3);
  normals->SetNumberOfTuples(1);
  double n[3] = { 0, 0, 0 };
  normals->SetTuple(0, n);

  this->FocalData = vtkSmartPointer<vtkPolyData>::New();
  this->FocalData->SetPoints(this->FocalPoint);
  this->FocalData->GetPointData()->SetNormals(normals);
  normals->Delete();

  this->Glypher = vtkSmartPointer<vtkGlyph3D>::New();
  this->Glypher->SetInputData(this->FocalData);
  this->Glypher->SetVectorModeToUseNormal();
  this->Glypher->OrientOn();
  this->Glypher->ScalingOn();
  this->Glypher->SetScaleModeToDataScalingOff();
  this->Glypher->SetScaleFactor(1.0);

  // The transformation of the cursor will be done via vtkGlyph3D
  // By default a vtkCursor2D will be used to define the cursor shape
  auto cursor2D = vtkSmartPointer<vtkCursor2D>::New();
  cursor2D->AllOff();
  cursor2D->PointOn();
  cursor2D->Update();

  this->Glypher->SetSourceData(cursor2D->GetOutput());

  this->ActiveSource = vtkSmartPointer<vtkSphereSource>::New();
  this->ActiveSource->SetThetaResolution(12);
  this->ActiveSource->SetRadius(0.5);
  this->ActiveSource->SetCenter(0, 0, 0);

  this->Mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  this->Mapper->SetInputData(this->Glypher->GetOutput());
  this->Mapper->SetResolveCoincidentTopologyToPolygonOffset();
  this->Mapper->ScalarVisibilityOff();
  this->Mapper->ImmediateModeRenderingOn();

  this->ActiveMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  this->ActiveMapper->SetInputData(this->ActiveSource->GetOutput());
  this->ActiveMapper->SetResolveCoincidentTopologyToPolygonOffset();
  this->ActiveMapper->ScalarVisibilityOff();
  this->ActiveMapper->ImmediateModeRenderingOn();

  // Set up the initial properties
  this->CreateDefaultProperties();

  this->Actor = vtkSmartPointer<vtkActor>::New();
  this->Actor->SetMapper(this->Mapper);
  this->Actor->SetProperty(this->Property);

  this->ActiveActor = vtkSmartPointer<vtkActor>::New();
  this->ActiveActor->SetMapper(this->ActiveMapper);
  this->ActiveActor->SetProperty(this->ActiveProperty);

  this->Lines = vtkSmartPointer<vtkPolyData>::New();
  this->LinesMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  this->LinesMapper->SetInputData(this->Lines);

  this->LinesActor = vtkSmartPointer<vtkActor>::New();
  this->LinesActor->SetMapper(this->LinesMapper);
  this->LinesActor->SetProperty(this->LinesProperty);

  this->InteractionOffset[0] = 0.0;
  this->InteractionOffset[1] = 0.0;

  this->AlwaysOnTop = 0;
}

//----------------------------------------------------------------------------
vtkPlaneContourRepresentationGlyph::~vtkPlaneContourRepresentationGlyph()
{
  this->ActiveProperty->Delete();
  this->LinesProperty->Delete();
  this->Property->Delete();

  this->UseContourPolygon(false);
}

//----------------------------------------------------------------------------
void vtkPlaneContourRepresentationGlyph::SetRenderer(vtkRenderer *ren)
{
  this->Superclass::SetRenderer(ren);
}

//----------------------------------------------------------------------------
int vtkPlaneContourRepresentationGlyph::ComputeInteractionState(int X, int Y, int vtkNotUsed(modified))
{
  double pos[4], xyz[3];
  this->FocalPoint->GetPoint(0, pos);
  pos[3] = 1.0;
  this->Renderer->SetWorldPoint(pos);
  this->Renderer->WorldToDisplay();
  this->Renderer->GetDisplayPoint(pos);

  xyz[0] = static_cast<double>(X);
  xyz[1] = static_cast<double>(Y);
  xyz[2] = pos[2];

  this->VisibilityOn();
  double tol2 = this->PixelTolerance * this->PixelTolerance;
  if (vtkMath::Distance2BetweenPoints(xyz, pos) <= tol2)
  {
    this->InteractionState = vtkPlaneContourRepresentationGlyph::Nearby;
  }
  else
  {
    if (this->ActiveNode != -1)
    {
      this->InteractionState = vtkPlaneContourRepresentation::NearPoint;
    }
    else
    {
      if (this->FindClosestDistanceToContour(X, Y) <= this->PixelTolerance)
      {
        this->InteractionState = vtkPlaneContourRepresentation::NearContour;
      }
      else
      {
        if (!this->ClosedLoop)
        {
          this->InteractionState = vtkPlaneContourRepresentationGlyph::Outside;
        }
        else
        {
          if (!this->ShootingAlgorithm(X, Y))
          {
            this->InteractionState = vtkPlaneContourRepresentationGlyph::Outside;
          }
          else
          {
            // checking the active node allow better node picking, even being inside the polygon
            if (-1 == this->ActiveNode)
            {
              this->InteractionState = vtkPlaneContourRepresentation::Inside;
            }
            else
            {
              this->InteractionState = vtkPlaneContourRepresentationGlyph::Outside;
            }
          }
        }
      }
    }
  }
  return this->InteractionState;
}

//----------------------------------------------------------------------------
// Record the current event position, and the rectilinear wipe position.
void vtkPlaneContourRepresentationGlyph::StartWidgetInteraction(double startEventPos[2])
{
  this->StartEventPosition[0] = startEventPos[0];
  this->StartEventPosition[1] = startEventPos[1];
  this->StartEventPosition[2] = 0.0;

  this->LastEventPosition[0] = startEventPos[0];
  this->LastEventPosition[1] = startEventPos[1];

  // How far is this in pixels from the position of this widget?
  // Maintain this during interaction such as translating (don't
  // force center of widget to snap to mouse position)

  // convert position to display coordinates
  double pos[2];
  this->GetNthNodeDisplayPosition(this->ActiveNode, pos);

  this->InteractionOffset[0] = pos[0] - startEventPos[0];
  this->InteractionOffset[1] = pos[1] - startEventPos[1];
}

//----------------------------------------------------------------------------
// Based on the displacement vector (computed in display coordinates) and
// the cursor state (which corresponds to which part of the widget has been
// selected), the widget points are modified.
// First construct a local coordinate system based on the display coordinates
// of the widget.
void vtkPlaneContourRepresentationGlyph::WidgetInteraction(double eventPos[2])
{
  // Process the motion
  if (this->CurrentOperation == vtkPlaneContourRepresentation::Translate)
  {
    this->Translate(eventPos);
  }

  if (this->CurrentOperation == vtkPlaneContourRepresentation::Shift)
  {
    this->ShiftContour(eventPos);
  }

  if (this->CurrentOperation == vtkPlaneContourRepresentation::Scale)
  {
    this->ScaleContour(eventPos);
  }

  // Bookeeping
  this->LastEventPosition[0] = eventPos[0];
  this->LastEventPosition[1] = eventPos[1];
}

//----------------------------------------------------------------------------
// Translate everything
void vtkPlaneContourRepresentationGlyph::Translate(double eventPos[2])
{
  double ref[3];

  if (!this->GetActiveNodeWorldPosition(ref)) return;

  double displayPos[2];
  displayPos[0] = eventPos[0] + this->InteractionOffset[0];
  displayPos[1] = eventPos[1] + this->InteractionOffset[1];

  double worldPos[3];
  double worldOrient[9] =
  { 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0 };
  if (this->PointPlacer->ComputeWorldPosition(this->Renderer, displayPos, ref, worldPos, worldOrient))
  {
    this->GetActiveNodeWorldPosition(ref);
    worldPos[normalCoordinateIndex(this->Orientation)] = this->Slice + this->PlaneShift;
    this->SetActiveNodeToWorldPosition(worldPos, worldOrient);

    // take back movement if it breaks the contour
    if (this->CheckContourIntersection(this->ActiveNode))
    {
      this->SetActiveNodeToWorldPosition(ref, worldOrient);
    }
  }
  else
  {
    // I really want to track the closest point here,
    // but I am postponing this at the moment....
  }
}

//----------------------------------------------------------------------------
void vtkPlaneContourRepresentationGlyph::ShiftContour(double eventPos[2])
{
  double vector[3];
  vector[0] = eventPos[0];
  vector[1] = eventPos[1];
  vector[2] = 0.0;

  double worldPos[3], worldPos2[3];
  double worldOrient[9] = { 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0 };
  this->Renderer->SetDisplayPoint(vector);
  this->Renderer->DisplayToWorld();
  this->Renderer->GetWorldPoint(worldPos);

  vector[0] = this->LastEventPosition[0];
  vector[1] = this->LastEventPosition[1];
  vector[2] = 0.0;

  this->Renderer->SetDisplayPoint(vector);
  this->Renderer->DisplayToWorld();
  this->Renderer->GetWorldPoint(worldPos2);

  vector[0] = worldPos[0] - worldPos2[0];
  vector[1] = worldPos[1] - worldPos2[1];
  vector[2] = worldPos[2] - worldPos2[2];
  vector[normalCoordinateIndex(this->Orientation)] = 0; // don't want a shift in the orientation plane.

  for (int i = 0; i < this->GetNumberOfNodes(); i++)
  {
    this->GetNthNodeWorldPosition(i, worldPos);
    worldPos[0] += vector[0];
    worldPos[1] += vector[1];
    worldPos[2] += vector[2];
    this->SetNthNodeWorldPosition(i, worldPos, worldOrient);
  }
  this->NeedToRenderOn();
}

//----------------------------------------------------------------------------
// not used
void vtkPlaneContourRepresentationGlyph::ScaleContour(double eventPos[2])
{
  double ref[3];

  if (!this->GetActiveNodeWorldPosition(ref))
    return;

  double centroid[3];
  ComputeCentroid(centroid);

  double r2 = vtkMath::Distance2BetweenPoints(ref, centroid);

  double displayPos[2];
  displayPos[0] = eventPos[0] + this->InteractionOffset[0];
  displayPos[1] = eventPos[1] + this->InteractionOffset[1];

  double worldPos[3];
  double worldOrient[9] = { 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0 };
  if (this->PointPlacer->ComputeWorldPosition(this->Renderer, displayPos, ref, worldPos, worldOrient))
  {
    double d2 = vtkMath::Distance2BetweenPoints(worldPos, centroid);
    if (d2 != 0.)
    {
      double ratio = sqrt(d2 / r2);

      for (int i = 0; i < this->GetNumberOfNodes(); i++)
      {
        this->GetNthNodeWorldPosition(i, ref);
        worldPos[0] = centroid[0] + ratio * (ref[0] - centroid[0]);
        worldPos[1] = centroid[0] + ratio * (ref[1] - centroid[1]);
        worldPos[2] = centroid[0] + ratio * (ref[2] - centroid[2]);
        this->SetNthNodeWorldPosition(i, worldPos, worldOrient);
      }
    }
  }
}

//----------------------------------------------------------------------------
void vtkPlaneContourRepresentationGlyph::ComputeCentroid(double* ioCentroid)
{
  double p[3];
  ioCentroid[0] = 0.;
  ioCentroid[1] = 0.;
  ioCentroid[2] = 0.;

  for (int i = 0; i < this->GetNumberOfNodes(); i++)
  {
    this->GetNthNodeWorldPosition(i, p);
    ioCentroid[0] += p[0];
    ioCentroid[1] += p[1];
    ioCentroid[2] += p[2];
  }
  double inv_N = 1. / static_cast<double>(this->GetNumberOfNodes());
  ioCentroid[0] *= inv_N;
  ioCentroid[1] *= inv_N;
  ioCentroid[2] *= inv_N;
}

//----------------------------------------------------------------------------
void vtkPlaneContourRepresentationGlyph::Scale(double eventPos[2])
{
  // Get the current scale factor
  double sf = this->Glypher->GetScaleFactor();

  // Compute the scale factor
  int *size = this->Renderer->GetSize();
  double dPos = static_cast<double>(eventPos[1] - this->LastEventPosition[1]);
  sf *= (1.0 + 2.0 * (dPos / size[1])); //scale factor of 2.0 is arbitrary

  // Scale the handle
  this->Glypher->SetScaleFactor(sf);
}

//----------------------------------------------------------------------------
void vtkPlaneContourRepresentationGlyph::CreateDefaultProperties()
{
  this->Property = vtkProperty::New();
  this->Property->SetColor(1.0, 1.0, 1.0);
  this->Property->SetLineWidth(0.5);
  this->Property->SetPointSize(4);

  this->ActiveProperty = vtkProperty::New();
  this->ActiveProperty->SetColor(1.0, 1.0, 1.0);
  this->ActiveProperty->SetRepresentationToSurface();
  this->ActiveProperty->SetAmbient(1.0);
  this->ActiveProperty->SetDiffuse(0.0);
  this->ActiveProperty->SetSpecular(0.0);
  this->ActiveProperty->SetLineWidth(1.0);

  this->LinesProperty = vtkProperty::New();
  this->LinesProperty->SetAmbient(1.0);
  this->LinesProperty->SetDiffuse(0.0);
  this->LinesProperty->SetSpecular(0.0);
  this->LinesProperty->SetColor(1, 1, 1);
  this->LinesProperty->SetLineWidth(1);
}

//----------------------------------------------------------------------------
// NOTE: modified to make the representation a closed loop, but the value of closed loop is
// false until the used ends defining the contour, then its true.
void vtkPlaneContourRepresentationGlyph::BuildLines()
{
  auto points = vtkSmartPointer<vtkPoints>::New();
  auto lines = vtkSmartPointer<vtkCellArray>::New();

  int i, j;
  vtkIdType index = 0;

  int count = this->GetNumberOfNodes();
  for (i = 0; i < this->GetNumberOfNodes(); i++)
  {
    count += this->GetNumberOfIntermediatePoints(i);
  }

  points->SetNumberOfPoints(count);
  vtkIdType numLines;

  numLines = count + 1;

  if (numLines > 0)
  {
    vtkIdType *lineIndices = new vtkIdType[numLines];

    double pos[3];
    for (i = 0; i < this->GetNumberOfNodes(); i++)
    {
      // Add the node
      this->GetNthNodeWorldPosition(i, pos);
      points->InsertPoint(index, pos);
      lineIndices[index] = index;
      index++;

      int numIntermediatePoints = this->GetNumberOfIntermediatePoints(i);

      for (j = 0; j < numIntermediatePoints; j++)
      {
        this->GetIntermediatePointWorldPosition(i, j, pos);
        points->InsertPoint(index, pos);
        lineIndices[index] = index;
        index++;
      }
    }

    lineIndices[index] = 0;

    lines->InsertNextCell(numLines, lineIndices);
    delete[] lineIndices;
  }

  this->Lines->SetPoints(points);
  this->Lines->SetLines(lines);
}

//----------------------------------------------------------------------------
vtkPolyData *vtkPlaneContourRepresentationGlyph::GetContourRepresentationAsPolyData()
{
  vtkPolyData *polyData = nullptr;

  if(this->Lines != nullptr && this->Lines->GetNumberOfLines() != 0 && this->ClosedLoop)
  {
    polyData = vtkPolyData::New();
    polyData->DeepCopy(this->Lines);

    for(vtkIdType i = 0; i < polyData->GetNumberOfPoints(); ++i)
    {
      double worldPos[3];
      polyData->GetPoints()->GetPoint(i, worldPos);
      worldPos[normalCoordinateIndex(this->Orientation)] = this->Slice;
      polyData->GetPoints()->SetPoint(i, worldPos);
    }

    polyData->GetPoints()->Modified();
    polyData->Modified();
  }

  return polyData;
}

//----------------------------------------------------------------------------
void vtkPlaneContourRepresentationGlyph::BuildRepresentation()
{
  // Make sure we are up to date with any changes made in the placer
  this->UpdateContour();

  double p1[4], p2[4];
  this->Renderer->GetActiveCamera()->GetFocalPoint(p1);
  p1[3] = 1.0;
  this->Renderer->SetWorldPoint(p1);
  this->Renderer->WorldToView();
  this->Renderer->GetViewPoint(p1);

  double depth = p1[2];
  double aspect[2];
  this->Renderer->ComputeAspect();
  this->Renderer->GetAspect(aspect);

  p1[0] = -aspect[0];
  p1[1] = -aspect[1];
  this->Renderer->SetViewPoint(p1);
  this->Renderer->ViewToWorld();
  this->Renderer->GetWorldPoint(p1);

  p2[0] = aspect[0];
  p2[1] = aspect[1];
  p2[2] = depth;
  p2[3] = 1.0;
  this->Renderer->SetViewPoint(p2);
  this->Renderer->ViewToWorld();
  this->Renderer->GetWorldPoint(p2);

  double distance = sqrt(vtkMath::Distance2BetweenPoints(p1, p2));

  int *size = this->Renderer->GetRenderWindow()->GetSize();
  double viewport[4];
  this->Renderer->GetViewport(viewport);

  double x, y, scale;

  x = size[0] * (viewport[2] - viewport[0]);
  y = size[1] * (viewport[3] - viewport[1]);

  scale = sqrt(x * x + y * y);

  distance = 1000 * distance / scale;

  int numPoints = this->GetNumberOfNodes();

  if (this->ActiveNode >= 0 && this->ActiveNode < this->GetNumberOfNodes())
  {
    this->FocalPoint->SetNumberOfPoints(numPoints - 1);
    this->FocalData->GetPointData()->GetNormals()->SetNumberOfTuples(numPoints - 1);
  }
  else
  {
    this->FocalPoint->SetNumberOfPoints(numPoints);
    this->FocalData->GetPointData()->GetNormals()->SetNumberOfTuples(numPoints);
  }
  int idx = 0;
  for (auto i = 0; i < numPoints; i++)
  {
    if (i != this->ActiveNode)
    {
      double worldPos[3];
      double worldOrient[9];
      this->GetNthNodeWorldPosition(i, worldPos);
      this->GetNthNodeWorldOrientation(i, worldOrient);
      this->FocalPoint->SetPoint(idx, worldPos);
      this->FocalData->GetPointData()->GetNormals()->SetTuple(idx, worldOrient + 6);
      idx++;
    }
  }

  this->FocalPoint->Modified();
  this->FocalData->GetPointData()->GetNormals()->Modified();
  this->FocalData->Modified();
  this->Glypher->SetScaleFactor(distance * this->HandleSize);
  this->Glypher->Update();
  this->Mapper->Update();

  if ((this->ActiveNode >= 0) && (this->ActiveNode < this->GetNumberOfNodes()))
  {
    double worldPos[3];
    this->GetNthNodeWorldPosition(this->ActiveNode, worldPos);
    this->ActiveSource->SetCenter(worldPos);
    this->ActiveSource->SetRadius(distance * this->HandleSize * 0.6);
    this->ActiveSource->Update();
    this->ActiveMapper->SetInputData(this->ActiveSource->GetOutput());
    this->ActiveMapper->Update();
    this->ActiveActor->VisibilityOn();
  }
  else
  {
    this->ActiveActor->VisibilityOff();
  }
}

//----------------------------------------------------------------------------
void vtkPlaneContourRepresentationGlyph::GetActors(vtkPropCollection *pc)
{
  this->Actor->GetActors(pc);
  this->ActiveActor->GetActors(pc);
  this->LinesActor->GetActors(pc);
}

//----------------------------------------------------------------------------
void vtkPlaneContourRepresentationGlyph::ReleaseGraphicsResources(vtkWindow *win)
{
  this->Actor->ReleaseGraphicsResources(win);
  this->ActiveActor->ReleaseGraphicsResources(win);
  this->LinesActor->ReleaseGraphicsResources(win);
}

//----------------------------------------------------------------------------
int vtkPlaneContourRepresentationGlyph::RenderOverlay(vtkViewport *viewport)
{
  int count = 0;
  count += this->LinesActor->RenderOverlay(viewport);
  if (this->Actor->GetVisibility())
  {
    count += this->Actor->RenderOverlay(viewport);
  }

  if (this->ActiveActor->GetVisibility())
  {
    count += this->ActiveActor->RenderOverlay(viewport);
  }

  return count;
}

//----------------------------------------------------------------------------
int vtkPlaneContourRepresentationGlyph::RenderOpaqueGeometry(vtkViewport *viewport)
{
  // Since we know RenderOpaqueGeometry gets called first, will do the build here
  this->BuildRepresentation();

  int count = 0;
  count += this->LinesActor->RenderOpaqueGeometry(viewport);
  if (this->Actor->GetVisibility())
  {
    count += this->Actor->RenderOpaqueGeometry(viewport);
  }
  if (this->ActiveActor->GetVisibility())
  {
    count += this->ActiveActor->RenderOpaqueGeometry(viewport);
  }

  return count;
}

//----------------------------------------------------------------------------
int vtkPlaneContourRepresentationGlyph::RenderTranslucentPolygonalGeometry(vtkViewport *viewport)
{
  int count = 0;
  count += this->LinesActor->RenderTranslucentPolygonalGeometry(viewport);
  if (this->Actor->GetVisibility())
  {
    count += this->Actor->RenderTranslucentPolygonalGeometry(viewport);
  }

  if (this->ActiveActor->GetVisibility())
  {
    count += this->ActiveActor->RenderTranslucentPolygonalGeometry(viewport);
  }

  return count;
}

//----------------------------------------------------------------------------
int vtkPlaneContourRepresentationGlyph::HasTranslucentPolygonalGeometry()
{
  int result = 0;
  result |= this->LinesActor->HasTranslucentPolygonalGeometry();
  if (this->Actor->GetVisibility())
  {
    result |= this->Actor->HasTranslucentPolygonalGeometry();
  }

  if (this->ActiveActor->GetVisibility())
  {
    result |= this->ActiveActor->HasTranslucentPolygonalGeometry();
  }

  return result;
}

//----------------------------------------------------------------------------
void vtkPlaneContourRepresentationGlyph::SetLineColor(double r, double g, double b)
{
  if (this->GetLinesProperty())
  {
    this->GetLinesProperty()->SetColor(r, g, b);
  }

  if(this->GetNumberOfNodes() != 0)
  {
    NeedToRenderOn();
  }
}

//----------------------------------------------------------------------------
double* vtkPlaneContourRepresentationGlyph::GetBounds()
{
  return this->Lines->GetPoints() ? this->Lines->GetPoints()->GetBounds() : nullptr;
}

//----------------------------------------------------------------------------
void vtkPlaneContourRepresentationGlyph::PrintSelf(ostream& os, vtkIndent indent)
{
  //Superclass typedef defined in vtkTypeMacro() found in vtkSetGet.h
  this->Superclass::PrintSelf(os, indent);

  os << indent << "Always On Top: " << (this->AlwaysOnTop ? "On\n" : "Off\n");

  if (this->Property)
  {
    os << indent << "Property: " << this->Property << "\n";
  }
  else
  {
    os << indent << "Property: (none)\n";
  }

  if (this->ActiveProperty)
  {
    os << indent << "Active Property: " << this->ActiveProperty << "\n";
  }
  else
  {
    os << indent << "Active Property: (none)\n";
  }

  if (this->LinesProperty)
  {
    os << indent << "Lines Property: " << this->LinesProperty << "\n";
  }
  else
  {
    os << indent << "Lines Property: (none)\n";
  }
}

//----------------------------------------------------------------------------
void vtkPlaneContourRepresentationGlyph::UseContourPolygon(bool value)
{
  if (this->useContourPolygon == value) return;

  QColor color;

  switch(value)
  {
    case true:
      if (this->Lines->GetPoints()->GetNumberOfPoints() < 3)
      {
        return;
      }

      this->m_polygonFilter = vtkSmartPointer<vtkContourToPolygonFilter>::New();
      this->m_polygonFilter->SetInputData(this->Lines);
      this->m_polygonFilter->SetReleaseDataFlag(true);
      this->m_polygonFilter->Update();

      this->m_polygonMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
      this->m_polygonMapper->SetInputConnection(this->m_polygonFilter->GetOutputPort());

      this->m_polygon = vtkSmartPointer<vtkActor>::New();
      this->m_polygon->SetMapper(this->m_polygonMapper);

      this->m_polygon->GetProperty()->SetColor(m_polygonColor.redF(), m_polygonColor.greenF(), m_polygonColor.blueF());
      this->m_polygon->GetProperty()->SetOpacity(m_polygonColor.alphaF());

      double position[3];
      this->m_polygon->GetPosition(position);
      position[normalCoordinateIndex(this->Orientation)] += this->PlaneShift;
      this->m_polygon->SetPosition(position);
      this->Renderer->AddActor(this->m_polygon);
      this->useContourPolygon = true;
      break;
    case false:
      if (this->m_polygon != nullptr)
      {
        this->Renderer->RemoveActor(this->m_polygon);
        this->m_polygonFilter = nullptr;
        this->m_polygonMapper = nullptr;
        this->m_polygon = nullptr;
        this->useContourPolygon = false;
      }
      break;
    default:
      break;
  }
}

//----------------------------------------------------------------------------
double vtkPlaneContourRepresentationGlyph::Distance2BetweenPoints(int displayPosX, int displayPosY, int node)
{
  double displayPos[2], nodePos[3], pointPos[3];
  displayPos[0] = displayPosX;
  displayPos[1] = displayPosY;

  this->GetNthNodeWorldPosition(node, nodePos);

  this->Renderer->SetDisplayPoint(displayPos);
  this->Renderer->DisplayToWorld();
  this->Renderer->GetWorldPoint(pointPos);

  // we're computing on a plane, duh!
  auto idx = normalCoordinateIndex(this->Orientation);
  nodePos[idx] = pointPos[idx] = 0;

  return vtkMath::Distance2BetweenPoints(pointPos, nodePos);
}

//----------------------------------------------------------------------------
void vtkPlaneContourRepresentationGlyph::setPolygonColor(const QColor &color)
{
  this->m_polygonColor = color;

  if (this->m_polygon)
  {
    this->m_polygon->GetProperty()->SetColor(m_polygonColor.redF(), m_polygonColor.greenF(), m_polygonColor.blueF());
    this->m_polygon->GetProperty()->SetOpacity(m_polygonColor.alphaF());
    this->m_polygon->Modified();
  }

  NeedToRenderOn();
}

//----------------------------------------------------------------------------
QColor vtkPlaneContourRepresentationGlyph::getPolygonColor() const
{
  return this->m_polygonColor;
}
