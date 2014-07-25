/*
 * vtkPlaneContourRepresentationGlyph.cpp
 *
 *  Created on: Sep 8, 2012
 *      Author: Felix de las Pozas Alvarez
 */
#include "vtkPlaneContourRepresentationGlyph.h"

#include "GUI/Pickers/ISelector.h"
#include <Core/ColorEngines/IColorEngine.h>
#include <Core/Model/Segmentation.h>

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

#include <QtGlobal>

using namespace ESPINA;

vtkStandardNewMacro(vtkPlaneContourRepresentationGlyph);

vtkPlaneContourRepresentationGlyph::vtkPlaneContourRepresentationGlyph()
{
  this->Property = NULL;
  this->ActiveProperty = NULL;
  this->LinesProperty = NULL;
  this->m_polygon = NULL;
  this->m_polygonFilter = NULL;
  this->m_polygonMapper = NULL;
  this->useContourPolygon = false;
  this->m_polygonColor = Qt::black;

  // Initialize state
  this->InteractionState = vtkPlaneContourRepresentation::Outside;

  this->Spacing[0] = 1.0;
  this->Spacing[1] = 1.0;
  this->CursorShape = NULL;
  this->ActiveCursorShape = NULL;
  this->HandleSize = 0.01;
  this->PointPlacer = vtkFocalPlanePointPlacer::New();
  this->LineInterpolator = vtkBezierContourLineInterpolator::New();

  // Represent the position of the cursor
  this->FocalPoint = vtkPoints::New();
  this->FocalPoint->SetNumberOfPoints(1);
  this->FocalPoint->SetPoint(0, 0.0, 0.0, 0.0);

  vtkDoubleArray *normals = vtkDoubleArray::New();
  normals->SetNumberOfComponents(3);
  normals->SetNumberOfTuples(100);
  normals->SetNumberOfTuples(1);
  double n[3] = { 0, 0, 0 };
  normals->SetTuple(0, n);

  // Represent the position of the cursor
  this->ActiveFocalPoint = vtkPoints::New();
  this->ActiveFocalPoint->SetNumberOfPoints(100);
  this->ActiveFocalPoint->SetNumberOfPoints(1);
  this->ActiveFocalPoint->SetPoint(0, 0.0, 0.0, 0.0);

  vtkDoubleArray *activeNormals = vtkDoubleArray::New();
  activeNormals->SetNumberOfComponents(3);
  activeNormals->SetNumberOfTuples(100);
  activeNormals->SetNumberOfTuples(1);
  activeNormals->SetTuple(0, n);

  this->FocalData = vtkPolyData::New();
  this->FocalData->SetPoints(this->FocalPoint);
  this->FocalData->GetPointData()->SetNormals(normals);
  normals->Delete();

  this->ActiveFocalData = vtkPolyData::New();
  this->ActiveFocalData->SetPoints(this->ActiveFocalPoint);
  this->ActiveFocalData->GetPointData()->SetNormals(activeNormals);
  activeNormals->Delete();

  this->Glypher = vtkGlyph3D::New();
  //TODO 2013-10-08 this->Glypher->SetInput(this->FocalData);
  this->Glypher->SetVectorModeToUseNormal();
  this->Glypher->OrientOn();
  this->Glypher->ScalingOn();
  this->Glypher->SetScaleModeToDataScalingOff();
  this->Glypher->SetScaleFactor(1.0);

  this->ActiveGlypher = vtkGlyph3D::New();
  //TODO 2013-10-08 this->ActiveGlypher->SetInput(this->ActiveFocalData);
  this->ActiveGlypher->SetVectorModeToUseNormal();
  this->ActiveGlypher->OrientOn();
  this->ActiveGlypher->ScalingOn();
  this->ActiveGlypher->SetScaleModeToDataScalingOff();
  this->ActiveGlypher->SetScaleFactor(1.0);

  // The transformation of the cursor will be done via vtkGlyph3D
  // By default a vtkCursor2D will be used to define the cursor shape
  vtkCursor2D *cursor2D = vtkCursor2D::New();
  cursor2D->AllOff();
  cursor2D->PointOn();
  cursor2D->Update();
  this->SetCursorShape(cursor2D->GetOutput());
  cursor2D->Delete();

  vtkSphereSource *sphere = vtkSphereSource::New();
  sphere->SetThetaResolution(12);
  sphere->SetRadius(0.5);
  sphere->SetCenter(0, 0, 0);

  vtkCleanPolyData* clean = vtkCleanPolyData::New();
  clean->PointMergingOn();
  clean->CreateDefaultLocator();
  clean->SetInputConnection(0, sphere->GetOutputPort(0));

  vtkTransform *t = vtkTransform::New();
  t->RotateZ(90.0);

  vtkTransformPolyDataFilter *tpd = vtkTransformPolyDataFilter::New();
  tpd->SetInputConnection(0, clean->GetOutputPort(0));
  tpd->SetTransform(t);
  clean->Delete();
  sphere->Delete();

  tpd->Update();
  this->SetActiveCursorShape(tpd->GetOutput());
  tpd->Delete();
  t->Delete();

  //TODO 2013-10-08 this->Glypher->SetSource(this->CursorShape);
  //TODO 2013-10-08 this->ActiveGlypher->SetSource(this->ActiveCursorShape);

  this->Mapper = vtkPolyDataMapper::New();
  //TODO 2013-10-08 this->Mapper->SetInput(this->Glypher->GetOutput());
  this->Mapper->SetResolveCoincidentTopologyToPolygonOffset();
  this->Mapper->ScalarVisibilityOff();
  this->Mapper->ImmediateModeRenderingOn();

  this->ActiveMapper = vtkPolyDataMapper::New();
  //TODO 2013-10-08 this->ActiveMapper->SetInput(this->ActiveGlypher->GetOutput());
  this->ActiveMapper->SetResolveCoincidentTopologyToPolygonOffset();
  this->ActiveMapper->ScalarVisibilityOff();
  this->ActiveMapper->ImmediateModeRenderingOn();

  // Set up the initial properties
  this->CreateDefaultProperties();

  this->Actor = vtkActor::New();
  this->Actor->SetMapper(this->Mapper);
  this->Actor->SetProperty(this->Property);

  this->ActiveActor = vtkActor::New();
  this->ActiveActor->SetMapper(this->ActiveMapper);
  this->ActiveActor->SetProperty(this->ActiveProperty);

  this->Lines = vtkPolyData::New();
  this->LinesMapper = vtkPolyDataMapper::New();
  //TODO 2013-10-08 this->LinesMapper->SetInput(this->Lines);

  this->LinesActor = vtkActor::New();
  this->LinesActor->SetMapper(this->LinesMapper);
  this->LinesActor->SetProperty(this->LinesProperty);

  this->InteractionOffset[0] = 0.0;
  this->InteractionOffset[1] = 0.0;

  this->AlwaysOnTop = 0;

  this->SelectedNodesPoints = NULL;
  this->SelectedNodesData = NULL;
  this->SelectedNodesCursorShape = NULL;
  this->SelectedNodesGlypher = NULL;
  this->SelectedNodesMapper = NULL;
  this->SelectedNodesActor = NULL;
}

vtkPlaneContourRepresentationGlyph::~vtkPlaneContourRepresentationGlyph()
{
  this->FocalPoint->Delete();
  this->FocalData->Delete();

  this->ActiveFocalPoint->Delete();
  this->ActiveFocalData->Delete();

  this->SetCursorShape(NULL);
  this->SetActiveCursorShape(NULL);

  this->Glypher->Delete();
  this->Mapper->Delete();
  this->Actor->Delete();

  this->ActiveGlypher->Delete();
  this->ActiveMapper->Delete();
  this->ActiveActor->Delete();

  this->Lines->Delete();
  this->LinesMapper->Delete();
  this->LinesActor->Delete();

  this->Property->Delete();
  this->ActiveProperty->Delete();
  this->LinesProperty->Delete();

  // Clear the selected nodes representation
  if (this->SelectedNodesPoints)
    this->SelectedNodesPoints->Delete();

  if (this->SelectedNodesData)
    this->SelectedNodesData->Delete();

  if (this->SelectedNodesCursorShape)
    this->SelectedNodesCursorShape->Delete();

  if (this->SelectedNodesGlypher)
    this->SelectedNodesGlypher->Delete();

  if (this->SelectedNodesMapper)
    this->SelectedNodesMapper->Delete();

  if (this->SelectedNodesActor)
    this->SelectedNodesActor->Delete();

  this->UseContourPolygon(false);
}

void vtkPlaneContourRepresentationGlyph::SetCursorShape(vtkPolyData *shape)
{
  if (shape != this->CursorShape)
  {
    if (this->CursorShape)
      this->CursorShape->Delete();

    this->CursorShape = shape;
    if (this->CursorShape)
      this->CursorShape->Register(this);

    //TODO 2013-10-08 if (this->CursorShape)
      //TODO 2013-10-08 this->Glypher->SetSource(this->CursorShape);

    this->Modified();
  }
}

vtkPolyData *vtkPlaneContourRepresentationGlyph::GetCursorShape()
{
	return this->CursorShape;
}

void vtkPlaneContourRepresentationGlyph::SetActiveCursorShape(vtkPolyData *shape)
{
  if (shape != this->ActiveCursorShape)
  {
    if (this->ActiveCursorShape)
      this->ActiveCursorShape->Delete();

    this->ActiveCursorShape = shape;
    if (this->ActiveCursorShape)
      this->ActiveCursorShape->Register(this);

    //TODO 2013-10-08 if (this->ActiveCursorShape)
      //TODO 2013-10-08 this->ActiveGlypher->SetSource(this->ActiveCursorShape);

    this->Modified();
  }
}

vtkPolyData *vtkPlaneContourRepresentationGlyph::GetActiveCursorShape()
{
  return this->ActiveCursorShape;
}

void vtkPlaneContourRepresentationGlyph::SetRenderer(vtkRenderer *ren)
{
  this->Superclass::SetRenderer(ren);
}

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
    if (!this->ActiveCursorShape)
      this->VisibilityOff();
  }
  else
  {
    if (this->ActiveNode != -1)
    {
      this->InteractionState = vtkPlaneContourRepresentation::NearPoint;
      if (!this->ActiveCursorShape)
        this->VisibilityOff();
    }
    else
    {
      if (this->FindClosestDistanceToContour(X, Y) <= this->PixelTolerance)
      {
        this->InteractionState = vtkPlaneContourRepresentation::NearContour;
        if (!this->ActiveCursorShape)
          this->VisibilityOff();
      }
      else
      {
        if (!this->ClosedLoop)
        {
          this->InteractionState = vtkPlaneContourRepresentationGlyph::Outside;
          if (!this->CursorShape)
            this->VisibilityOff();
        }
        else
        {
          if (!this->ShootingAlgorithm(X, Y))
          {
            this->InteractionState = vtkPlaneContourRepresentationGlyph::Outside;
            if (!this->CursorShape)
              this->VisibilityOff();
          }
          else
          {
            // checking the active node allow better node picking, even being inside the polygon
            if (-1 == this->ActiveNode)
              this->InteractionState = vtkPlaneContourRepresentation::Inside;
            else
              this->InteractionState = vtkPlaneContourRepresentationGlyph::Outside;
          }
        }
      }
    }
  }
  return this->InteractionState;
}

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

// Based on the displacement vector (computed in display coordinates) and
// the cursor state (which corresponds to which part of the widget has been
// selected), the widget points are modified.
// First construct a local coordinate system based on the display coordinates
// of the widget.
void vtkPlaneContourRepresentationGlyph::WidgetInteraction(double eventPos[2])
{
  // Process the motion
  if (this->CurrentOperation == vtkPlaneContourRepresentation::Translate)
    this->Translate(eventPos);

  if (this->CurrentOperation == vtkPlaneContourRepresentation::Shift)
    this->ShiftContour(eventPos);

  if (this->CurrentOperation == vtkPlaneContourRepresentation::Scale)
    this->ScaleContour(eventPos);

  // Book keeping
  this->LastEventPosition[0] = eventPos[0];
  this->LastEventPosition[1] = eventPos[1];
}

// Translate everything
void vtkPlaneContourRepresentationGlyph::Translate(double eventPos[2])
{
  double ref[3];

  if (!this->GetActiveNodeWorldPosition(ref))
    return;

  double displayPos[2];
  displayPos[0] = eventPos[0] + this->InteractionOffset[0];
  displayPos[1] = eventPos[1] + this->InteractionOffset[1];

  double worldPos[3];
  double worldOrient[9] =
  { 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0 };
  if (this->PointPlacer->ComputeWorldPosition(this->Renderer, displayPos, ref, worldPos, worldOrient))
  {
    this->GetActiveNodeWorldPosition(ref);
    switch(this->Orientation)
    {
      case AXIAL:
        worldPos[this->Orientation] = -0.1;
        break;
      case CORONAL:
      case SAGITTAL:
        worldPos[this->Orientation] = 0.1;
        break;
      default:
        Q_ASSERT(false);
        break;
    }
    this->SetActiveNodeToWorldPosition(worldPos, worldOrient);

    // take back movement if it breaks the contour
    if (this->CheckContourIntersection(this->ActiveNode))
      this->SetActiveNodeToWorldPosition(ref, worldOrient);
  }
  else
  {
    // I really want to track the closest point here,
    // but I am postponing this at the moment....
  }
}

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
  if (this->ShowSelectedNodes && this->SelectedNodesGlypher)
    this->SelectedNodesGlypher->SetScaleFactor(sf);
}

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

// NOTE: modified to make the representation a closed loop, but the value of closed loop is
// false until the used ends defining the contour, then its true.
void vtkPlaneContourRepresentationGlyph::BuildLines()
{
  vtkPoints *points = vtkPoints::New();
  vtkCellArray *lines = vtkCellArray::New();

  int i, j;
  vtkIdType index = 0;

  int count = this->GetNumberOfNodes();
  for (i = 0; i < this->GetNumberOfNodes(); i++)
    count += this->GetNumberOfIntermediatePoints(i);

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

  points->Delete();
  lines->Delete();
}

vtkPolyData *vtkPlaneContourRepresentationGlyph::GetContourRepresentationAsPolyData()
{
  // Get the points in this contour as a vtkPolyData.
  return this->Lines;
}

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

  this->Glypher->SetScaleFactor(distance * this->HandleSize);
  this->ActiveGlypher->SetScaleFactor(distance * this->HandleSize);
  int numPoints = this->GetNumberOfNodes();
  int i;
  if (this->ShowSelectedNodes && this->SelectedNodesGlypher)
  {
    this->SelectedNodesGlypher->SetScaleFactor(distance * this->HandleSize);
    this->FocalPoint->Reset();
    this->FocalPoint->SetNumberOfPoints(0);
    this->FocalData->GetPointData()->GetNormals()->SetNumberOfTuples(0);
    this->SelectedNodesPoints->Reset();
    this->SelectedNodesPoints->SetNumberOfPoints(0);
    this->SelectedNodesData->GetPointData()->GetNormals()->SetNumberOfTuples(0);
    for (i = 0; i < numPoints; i++)
    {
      if (i != this->ActiveNode)
      {
        double worldPos[3];
        double worldOrient[9];
        this->GetNthNodeWorldPosition(i, worldPos);
        this->GetNthNodeWorldOrientation(i, worldOrient);
        if (this->GetNthNodeSelected(i))
        {
          this->SelectedNodesPoints->InsertNextPoint(worldPos);
          this->SelectedNodesData->GetPointData()->GetNormals()->InsertNextTuple(worldOrient + 6);
        }
        else
        {
          this->FocalPoint->InsertNextPoint(worldPos);
          this->FocalData->GetPointData()->GetNormals()->InsertNextTuple(worldOrient + 6);
        }
      }
    }
    this->SelectedNodesPoints->Modified();
    this->SelectedNodesData->GetPointData()->GetNormals()->Modified();
    this->SelectedNodesData->Modified();
  }
  else
  {
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
    for (i = 0; i < numPoints; i++)
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
  }

  this->FocalPoint->Modified();
  this->FocalData->GetPointData()->GetNormals()->Modified();
  this->FocalData->Modified();

  if (this->ActiveNode >= 0 && this->ActiveNode < this->GetNumberOfNodes())
  {
    double worldPos[3];
    double worldOrient[9];
    this->GetNthNodeWorldPosition(this->ActiveNode, worldPos);
    this->GetNthNodeWorldOrientation(this->ActiveNode, worldOrient);
    this->ActiveFocalPoint->SetPoint(0, worldPos);
    this->ActiveFocalData->GetPointData()->GetNormals()->SetTuple(0, worldOrient + 6);

    this->ActiveFocalPoint->Modified();
    this->ActiveFocalData->GetPointData()->GetNormals()->Modified();
    this->ActiveFocalData->Modified();
    this->ActiveActor->VisibilityOn();
  }
  else
    this->ActiveActor->VisibilityOff();
}

void vtkPlaneContourRepresentationGlyph::GetActors(vtkPropCollection *pc)
{
  this->Actor->GetActors(pc);
  this->ActiveActor->GetActors(pc);
  this->LinesActor->GetActors(pc);
  if (this->ShowSelectedNodes && this->SelectedNodesActor)
    this->SelectedNodesActor->GetActors(pc);
}

void vtkPlaneContourRepresentationGlyph::ReleaseGraphicsResources(vtkWindow *win)
{
  this->Actor->ReleaseGraphicsResources(win);
  this->ActiveActor->ReleaseGraphicsResources(win);
  this->LinesActor->ReleaseGraphicsResources(win);
}

int vtkPlaneContourRepresentationGlyph::RenderOverlay(vtkViewport *viewport)
{
  int count = 0;
  count += this->LinesActor->RenderOverlay(viewport);
  if (this->Actor->GetVisibility())
    count += this->Actor->RenderOverlay(viewport);
  if (this->ActiveActor->GetVisibility())
    count += this->ActiveActor->RenderOverlay(viewport);
  return count;
}

int vtkPlaneContourRepresentationGlyph::RenderOpaqueGeometry(vtkViewport *viewport)
{
  // Since we know RenderOpaqueGeometry gets called first, will do the build here
  this->BuildRepresentation();

  GLboolean flag = GL_FALSE;
  if (this->AlwaysOnTop && (this->ActiveActor->GetVisibility() || this->LinesActor->GetVisibility()))
  {
    glGetBooleanv(GL_DEPTH_TEST, &flag);
    if (flag)
      glDisable(GL_DEPTH_TEST);
  }

  int count = 0;
  count += this->LinesActor->RenderOpaqueGeometry(viewport);
  if (this->Actor->GetVisibility())
    count += this->Actor->RenderOpaqueGeometry(viewport);
  if (this->ActiveActor->GetVisibility())
    count += this->ActiveActor->RenderOpaqueGeometry(viewport);
  if (this->ShowSelectedNodes && this->SelectedNodesActor
      && this->SelectedNodesActor->GetVisibility())
    count += this->SelectedNodesActor->RenderOpaqueGeometry(viewport);

  if (flag && this->AlwaysOnTop && (this->ActiveActor->GetVisibility() || this->LinesActor->GetVisibility()))
    glEnable(GL_DEPTH_TEST);

  return count;
}

int vtkPlaneContourRepresentationGlyph::RenderTranslucentPolygonalGeometry(vtkViewport *viewport)
{
  int count = 0;
  count += this->LinesActor->RenderTranslucentPolygonalGeometry(viewport);
  if (this->Actor->GetVisibility())
    count += this->Actor->RenderTranslucentPolygonalGeometry(viewport);
  if (this->ActiveActor->GetVisibility())
    count += this->ActiveActor->RenderTranslucentPolygonalGeometry(viewport);
  return count;
}

int vtkPlaneContourRepresentationGlyph::HasTranslucentPolygonalGeometry()
{
  int result = 0;
  result |= this->LinesActor->HasTranslucentPolygonalGeometry();
  if (this->Actor->GetVisibility())
    result |= this->Actor->HasTranslucentPolygonalGeometry();
  if (this->ActiveActor->GetVisibility())
    result |= this->ActiveActor->HasTranslucentPolygonalGeometry();

  return result;
}

void vtkPlaneContourRepresentationGlyph::SetLineColor(double r, double g, double b)
{
  if (this->GetLinesProperty())
    this->GetLinesProperty()->SetColor(r, g, b);
}

void vtkPlaneContourRepresentationGlyph::SetShowSelectedNodes(int flag)
{
  vtkDebugMacro(<< this->GetClassName() << " (" << this << "): setting ShowSelectedNodes to " << flag);
  if (this->ShowSelectedNodes != flag)
  {
    this->ShowSelectedNodes = flag;
    this->Modified();

    if (this->ShowSelectedNodes)
    {
      if (!this->SelectedNodesActor)
        this->CreateSelectedNodesRepresentation();
      else
        this->SelectedNodesActor->SetVisibility(1);
    }
    else
    {
      if (this->SelectedNodesActor)
        this->SelectedNodesActor->SetVisibility(0);
    }
  }
}

double* vtkPlaneContourRepresentationGlyph::GetBounds()
{
  return this->Lines->GetPoints() ? this->Lines->GetPoints()->GetBounds() : NULL;
}

void vtkPlaneContourRepresentationGlyph::CreateSelectedNodesRepresentation()
{
  vtkSphereSource *sphere = vtkSphereSource::New();
  sphere->SetThetaResolution(12);
  sphere->SetRadius(0.3);
  this->SelectedNodesCursorShape = sphere->GetOutput();
  this->SelectedNodesCursorShape->Register(this);
  sphere->Delete();

  // Represent the position of the cursor
  this->SelectedNodesPoints = vtkPoints::New();
  this->SelectedNodesPoints->SetNumberOfPoints(100);

  vtkDoubleArray *normals = vtkDoubleArray::New();
  normals->SetNumberOfComponents(3);
  normals->SetNumberOfTuples(100);
  normals->SetNumberOfTuples(1);
  double n[3] = { 0, 0, 0 };
  normals->SetTuple(0, n);

  this->SelectedNodesData = vtkPolyData::New();
  this->SelectedNodesData->SetPoints(this->SelectedNodesPoints);
  this->SelectedNodesData->GetPointData()->SetNormals(normals);
  normals->Delete();

  this->SelectedNodesGlypher = vtkGlyph3D::New();
  //TODO 2013-10-08 this->SelectedNodesGlypher->SetInput(this->SelectedNodesData);
  this->SelectedNodesGlypher->SetVectorModeToUseNormal();
  this->SelectedNodesGlypher->OrientOn();
  this->SelectedNodesGlypher->ScalingOn();
  this->SelectedNodesGlypher->SetScaleModeToDataScalingOff();
  this->SelectedNodesGlypher->SetScaleFactor(1.0);

  //TODO 2013-10-08 this->SelectedNodesGlypher->SetSource(this->SelectedNodesCursorShape);

  this->SelectedNodesMapper = vtkPolyDataMapper::New();
  //TODO 2013-10-08 this->SelectedNodesMapper->SetInput(this->SelectedNodesGlypher->GetOutput());
  this->SelectedNodesMapper->SetResolveCoincidentTopologyToPolygonOffset();
  this->SelectedNodesMapper->ScalarVisibilityOff();
  this->SelectedNodesMapper->ImmediateModeRenderingOn();

  vtkProperty* selProperty = vtkProperty::New();
  selProperty->SetColor(0.0, 1.0, 0.0);
  selProperty->SetLineWidth(0.5);
  selProperty->SetPointSize(3);

  this->SelectedNodesActor = vtkActor::New();
  this->SelectedNodesActor->SetMapper(this->SelectedNodesMapper);
  this->SelectedNodesActor->SetProperty(selProperty);
  selProperty->Delete();
}

void vtkPlaneContourRepresentationGlyph::PrintSelf(ostream& os, vtkIndent indent)
{
  //Superclass typedef defined in vtkTypeMacro() found in vtkSetGet.h
  this->Superclass::PrintSelf(os, indent);

  os << indent << "Always On Top: " << (this->AlwaysOnTop ? "On\n" : "Off\n");
  os << indent << "ShowSelectedNodes: " << this->ShowSelectedNodes << endl;

  if (this->Property)
    os << indent << "Property: " << this->Property << "\n";
  else
    os << indent << "Property: (none)\n";

  if (this->ActiveProperty)
    os << indent << "Active Property: " << this->ActiveProperty << "\n";
  else
    os << indent << "Active Property: (none)\n";

  if (this->LinesProperty)
    os << indent << "Lines Property: " << this->LinesProperty << "\n";
  else
    os << indent << "Lines Property: (none)\n";
}

void vtkPlaneContourRepresentationGlyph::UseContourPolygon(bool value)
{
  if (this->useContourPolygon == value)
    return;

  QColor color;

  switch(value)
  {
    case true:
      if (this->Lines->GetPoints()->GetNumberOfPoints() < 3)
        return;

      this->m_polygonFilter = vtkSmartPointer<vtkContourToPolygonFilter>::New();
      //TODO 2013-10-08 this->m_polygonFilter->SetInputConnection(this->Lines->GetProducerPort());
      this->m_polygonFilter->SetReleaseDataFlag(true);
      this->m_polygonFilter->Update();

      this->m_polygonMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
      this->m_polygonMapper->SetInputConnection(this->m_polygonFilter->GetOutputPort());

      this->m_polygon = vtkSmartPointer<vtkActor>::New();
      this->m_polygon->SetMapper(this->m_polygonMapper);

      this->m_polygon->GetProperty()->SetColor(m_polygonColor.redF(), m_polygonColor.greenF(), m_polygonColor.blueF());
      this->m_polygon->GetProperty()->SetOpacity(0.5);

      double position[3];
      this->m_polygon->GetPosition(position);
      switch(this->Orientation)
      {
        case AXIAL:
          position[this->Orientation] = -0.1;
          break;
        case CORONAL:
        case SAGITTAL:
          position[this->Orientation] = 0.1;
          break;
        default:
          Q_ASSERT(false);
          break;
      }
      this->m_polygon->SetPosition(position);
      this->Renderer->AddActor(this->m_polygon);
      this->useContourPolygon = true;
      break;
    case false:
      if (this->m_polygon != NULL)
      {
        this->Renderer->RemoveActor(this->m_polygon);
        this->m_polygonFilter = NULL;
        this->m_polygonMapper = NULL;
        this->m_polygon = NULL;
        this->useContourPolygon = false;
      }
      break;
    default:
      break;
  }
}

double vtkPlaneContourRepresentationGlyph::Distance2BetweenPoints(int displayPosX, int displayPosY, int node)
{
  double displayPos[2];
  displayPos[0] = displayPosX;
  displayPos[1] = displayPosY;

  double nodePos[3];
  this->GetNthNodeWorldPosition(node, nodePos);

  double pointPos[3];
  double worldOrient[9] = { 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0 };

  // Compute the world position from the display position based on the concrete representation's constraints
  // If this is not a valid display location return 0
  if (!this->PointPlacer->ComputeWorldPosition(this->Renderer, displayPos, pointPos, worldOrient))
    return 0;

  switch(this->Orientation)
  {
    case AXIAL:
      pointPos[this->Orientation] = -0.1;
      break;
    case CORONAL:
    case SAGITTAL:
      pointPos[this->Orientation] = 0.1;
      break;
    default:
      Q_ASSERT(false);
      break;
  }

  return ((pointPos[0] - nodePos[0]) * (pointPos[0] - nodePos[0])) + ((pointPos[1] - nodePos[1]) * (pointPos[1] - nodePos[1])) + ((pointPos[2] - nodePos[2]) * (pointPos[2] - nodePos[2]));
}

void vtkPlaneContourRepresentationGlyph::setPolygonColor(QColor color)
{
  this->m_polygonColor = color;
  if (this->m_polygon)
  {
    this->m_polygon->GetProperty()->SetColor(m_polygonColor.redF(), m_polygonColor.greenF(), m_polygonColor.greenF());
    this->m_polygon->GetProperty()->SetOpacity(0.5);
    this->m_polygon->Modified();
  }
}

QColor vtkPlaneContourRepresentationGlyph::getPolygonColor()
{
  return this->m_polygonColor;
}
