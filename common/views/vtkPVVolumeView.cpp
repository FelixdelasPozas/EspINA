/*
 *    <one line to give the program's name and a brief idea of what it does.>
 *    Copyright (C) 2011  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#include "vtkPVVolumeView.h"

#include <QDebug>
#include <assert.h>

#include <vtkAbstractPicker.h>
#include <vtkAxisActor2D.h>
#include <vtkCamera.h>
#include <vtkCellArray.h>
#include <vtkCommand.h>
#include <vtkDataRepresentation.h>
#include <vtkImageActor.h>
#include <vtkInteractorStyleImage.h>
#include <vtkLegendScaleActor.h>
#include <vtkMatrix4x4.h>
#include <vtkObjectFactory.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkProperty2D.h>
#include <vtkPVGenericRenderWindowInteractor.h>
#include <vtkPVInteractorStyle.h>
#include <vtkPVSynchronizedRenderWindows.h>
#include <vtkRenderer.h>
#include <vtkRenderViewBase.h>
#include <vtkRenderWindow.h>
#include <vtkSmartPointer.h>
#include <vtkTextProperty.h>

// // Interactor Style to be used with Volume Views
// class vtkInteractorStyleEspinaVolume
// : public vtkInteractorStyleImage
// {
// public:
//   static vtkInteractorStyleEspinaVolume *New();
//   vtkTypeMacro ( vtkInteractorStyleEspinaVolume,vtkInteractorStyleImage );
// 
//   // Disable mouse wheel
//   virtual void OnMouseWheelForward() {}
//   virtual void OnMouseWheelBackward() {}
// 
//   virtual void OnLeftButtonDown() {}
//   virtual void OnLeftButtonUp() {}
//   //   virtual void OnMouseMove();
// protected:
//   explicit vtkInteractorStyleEspinaVolume();
//   virtual ~vtkInteractorStyleEspinaVolume();
// 
// private:
//   vtkInteractorStyleEspinaVolume ( const vtkInteractorStyleEspinaVolume& ); // Not implemented
//   void operator= ( const vtkInteractorStyleEspinaVolume& );        // Not implemented
// };

// vtkStandardNewMacro ( vtkInteractorStyleEspinaVolume );
// 
// //-----------------------------------------------------------------------------
// vtkInteractorStyleEspinaVolume::vtkInteractorStyleEspinaVolume()
// {
// }
// 
// //-----------------------------------------------------------------------------
// vtkInteractorStyleEspinaVolume::~vtkInteractorStyleEspinaVolume()
// {
//   qDebug() << "vtkInteractorStyleEspinaVolume(" << this << "): Destroyed";
// }

//----------------------------------------------------------------------------
// vtkEspinaView
//----------------------------------------------------------------------------
vtkStandardNewMacro ( vtkPVVolumeView );

//----------------------------------------------------------------------------
vtkPVVolumeView::vtkPVVolumeView()
: m_pendingActor(NULL)
{
  memset(Crosshair, 0, 3*sizeof(double));
  memset(Focus, 0, 3*sizeof(double));

  this->SetCenterAxesVisibility ( false );
  this->SetOrientationAxesVisibility ( false );
  this->SetOrientationAxesInteractivity ( false );
  this->SetInteractionMode ( INTERACTION_MODE_3D );

  memset(AxialSlice, 0, 16*sizeof(double));
  AxialSlice[0] = AxialSlice[5] = AxialSlice[10] = AxialSlice[15] = 1;
  AxialMatrix = vtkMatrix4x4::New();
  AxialMatrix->DeepCopy(AxialSlice);

  memset(SagittalSlice, 0, 16*sizeof(double));
  SagittalSlice[4] = SagittalSlice[15] = 1;
  SagittalSlice[2] = SagittalSlice[9] = -1;
  SagittalMatrix = vtkMatrix4x4::New();
  SagittalMatrix->DeepCopy(SagittalSlice);

  memset(CoronalSlice, 0, 16*sizeof(double));
  CoronalSlice[0] = CoronalSlice[6] = CoronalSlice[15] = 1;
  CoronalSlice[9] = -1;
  CoronalMatrix = vtkMatrix4x4::New();
  CoronalMatrix->DeepCopy(CoronalSlice);

//   if ( this->Interactor )
//   {
    //     vtkInteractorStyleImage *style = vtkInteractorStyleImage::New();
//     vtkInteractorStyleEspinaVolume *style = vtkInteractorStyleEspinaVolume::New();
// vtkInteractorStyleTrackballCamera *style = vtkInteractorStyleTrackballCamera::New();
//     this->Interactor->SetInteractorStyle ( style );
//     style->Delete();
//   }

  RenderView->GetRenderer()->SetLayer(0);
  NonCompositedRenderer->SetLayer(1);
//   this->OverviewRenderer = vtkSmartPointer<vtkRenderer>::New();
//   this->OverviewRenderer->SetViewport(0.75, 0, 1, 0.25);
//   OverviewRenderer->SetLayer(2);
//   this->GetRenderWindow()->AddRenderer(this->OverviewRenderer);

//   initCrosshairs();
//   initRuler();
  //     qDebug() << "vtkPVVolumeView("<< this << "): Created";
}

//----------------------------------------------------------------------------
void vtkPVVolumeView::initCrosshairs()
{
  vtkSmartPointer<vtkPoints> HPoints = vtkSmartPointer<vtkPoints>::New();
  HPoints->InsertNextPoint(LastComputedBounds[0], 0, 0);
  HPoints->InsertNextPoint(LastComputedBounds[1], 0, 0);
  vtkSmartPointer<vtkCellArray> HLine = vtkSmartPointer<vtkCellArray>::New();
  HLine->EstimateSize(1, 2);
  HLine->InsertNextCell (2);
  HLine->InsertCellPoint(0);
  HLine->InsertCellPoint(1);

  HCrossLineData = vtkSmartPointer<vtkPolyData>::New();
  HCrossLineData->SetPoints(HPoints);
  HCrossLineData->SetLines (HLine);
  HCrossLineData->Update();

  vtkSmartPointer<vtkPolyDataMapper> HMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  HMapper->SetInput(HCrossLineData);

  HCrossLine = vtkSmartPointer<vtkActor>::New();
  HCrossLine->SetMapper(HMapper);
  HCrossLine->SetPickable(false);
  //   HCrossLine->GetProperty()->SetLineStipplePattern(0xF0F0);

  NonCompositedRenderer->AddActor(HCrossLine);
//   OverviewRenderer->AddActor(HCrossLine);

  vtkSmartPointer<vtkPoints> VPoints = vtkSmartPointer<vtkPoints>::New();
  VPoints->InsertNextPoint(0, LastComputedBounds[2], 0);
  VPoints->InsertNextPoint(0, LastComputedBounds[3], 0);
  vtkSmartPointer<vtkCellArray> VLine = vtkSmartPointer<vtkCellArray>::New();
  VLine->EstimateSize(1, 2);
  VLine->InsertNextCell (2);
  VLine->InsertCellPoint(0);
  VLine->InsertCellPoint(1);

  VCrossLineData = vtkSmartPointer<vtkPolyData>::New();
  VCrossLineData->SetPoints(VPoints);
  VCrossLineData->SetLines(VLine);
  VCrossLineData->Update();

  vtkSmartPointer<vtkPolyDataMapper> VMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  VMapper->SetInput(VCrossLineData);

  VCrossLine = vtkSmartPointer<vtkActor>::New();
  VCrossLine->SetMapper(VMapper);
  VCrossLine->SetPickable(false);
  //   VCrossLine->GetProperty()->SetLineStipplePattern(0xF0F0);

  NonCompositedRenderer->AddActor(VCrossLine);
//   OverviewRenderer->AddActor(VCrossLine);

  //   vtkSmartPointer<vtkPoints> BPoints = vtkSmartPointer<vtkPoints>::New();
  //   BPoints->InsertNextPoint(LastComputedBounds[0],LastComputedBounds[2],0);
  //   BPoints->InsertNextPoint(LastComputedBounds[0],LastComputedBounds[3],0);
  //   BPoints->InsertNextPoint(LastComputedBounds[1],LastComputedBounds[3],0);
  //   BPoints->InsertNextPoint(LastComputedBounds[1],LastComputedBounds[2],0);
  //   vtkSmartPointer<vtkCellArray> BLine = vtkSmartPointer<vtkCellArray>::New();
  //   BLine->EstimateSize(4,2);
  //   BLine->InsertNextCell(2);
  //   BLine->InsertCellPoint(0);
  //   BLine->InsertCellPoint(1);
  //   BLine->InsertNextCell(2);
  //   BLine->InsertCellPoint(1);
  //   BLine->InsertCellPoint(2);
  //   BLine->InsertNextCell(2);
  //   BLine->InsertCellPoint(2);
  //   BLine->InsertCellPoint(3);
  //   BLine->InsertNextCell(2);
  //   BLine->InsertCellPoint(3);
  //   BLine->InsertCellPoint(0);
  //
  //   BorderData = vtkSmartPointer<vtkPolyData>::New();
  //   BorderData->SetPoints(BPoints);
  //   BorderData->SetLines(BLine);
  //   BorderData->Update();
  //
  //   vtkSmartPointer<vtkPolyDataMapper> BMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  //   BMapper->SetInput(BorderData);
  //
  //   Border = vtkSmartPointer<vtkActor>::New();
  //   Border->SetMapper(BMapper);
  //   Border->SetPickable(false);
  //   //   HCrossLine->GetProperty()->SetLineStipplePattern(0xF0F0);
  //
  //   NonCompositedRenderer->AddActor(Border);
  //   OverviewRenderer->AddActor(Border);

}

//----------------------------------------------------------------------------
void vtkPVVolumeView::initRuler()
{
  RulerSize[0] = 150;
  RulerSize[1] = 2;
  Ruler = vtkSmartPointer<vtkAxisActor2D>::New();
  Ruler->SetPosition ( 0.02,0.98 );
  Ruler->SetPosition2 ( 0.15,0.98 );
  Ruler->SetPickable ( false );
  Ruler->SetLabelFactor ( 0.8 );
  Ruler->SetFontFactor ( 1 );
  Ruler->SetTitle ( "nm" );
  Ruler->SetAdjustLabels ( false );
  Ruler->SetNumberOfLabels ( 3 );
  this->NonCompositedRenderer->AddActor ( Ruler );
  //   RulerRenderer->ResetCamera();
}


//----------------------------------------------------------------------------
vtkPVVolumeView::~vtkPVVolumeView()
{
  //   qDebug() << "vtkPVVolumeView("<< this << "): Deleted";
}

//----------------------------------------------------------------------------
void vtkPVVolumeView::PrintSelf ( ostream& os, vtkIndent indent )
{
  this->Superclass::PrintSelf ( os, indent );
  os << indent << "OverviewRenderer: ";
//   if ( this->OverviewRenderer )
//   {
//     os << "\n";
//     this->OverviewRenderer->PrintSelf ( os, indent.GetNextIndent() );
//   }
}

// //----------------------------------------------------------------------------
// vtkRenderer* vtkPVVolumeView::GetOverviewRenderer()
// {
//   return OverviewRenderer.GetPointer();
// }

//----------------------------------------------------------------------------
void vtkPVVolumeView::AddActor(VolumeActor *actor)
{
  //   qDebug() << "Add Actor";
  bool in_cave_mode = this->SynchronizedWindows->GetIsInCave();
  if ( in_cave_mode && !this->GetRemoteRenderingAvailable() )
  {
    static bool warned_once = false;
    if ( !warned_once )
    {
      vtkErrorMacro (
	"In Cave mode and Display cannot be opened on server-side! "
	"Ensure the environment is set correctly in the pvx file." );
      in_cave_mode = true;
    }
  }

  // Decide if we are doing remote rendering or local rendering.
  bool using_distributed_rendering = in_cave_mode || this->GetUseDistributedRendering();
  if ( this->GetLocalProcessDoesRendering (using_distributed_rendering))
    RenderView->GetInteractor()->GetPicker()->AddPickList(actor->prop);

  RenderView->GetRenderer()->AddActor(actor->prop);
//   OverviewRenderer->AddActor(actor->prop);
  SetCrosshair(Crosshair);
  m_actors << actor;
  Q_ASSERT(!m_pendingActor);
  m_pendingActor = actor;
  double bounds[6] = {0,500,0,500,0,200};
}

//----------------------------------------------------------------------------
void vtkPVVolumeView::RemoveActor(VolumeActor *actor)
{
  RenderView->GetRenderer()->RemoveActor(actor->prop);
//   OverviewRenderer->RemoveActor(actor->prop);
  m_actors.removeOne(actor);
  if (Segmentations.contains(actor))
    Segmentations.removeOne(actor);
  if (Channels.contains(actor))
    Channels.removeOne(actor);
}

//----------------------------------------------------------------------------
void vtkPVVolumeView::AddChannel(VolumeActor* actor)
{
  AddActor(actor);
  Channels.append(actor);
}

//----------------------------------------------------------------------------
void vtkPVVolumeView::AddSegmentation(vtkPVVolumeView::VolumeActor* actor)
{
  //   qDebug() << "Add Segmentation";
  AddActor(actor);
  actor->prop->SetVisibility(ShowSegmentations);
  double pos[3];
  actor->prop->GetPosition(pos);
  pos[2] = pos[2] - 0.2;
  actor->prop->SetPosition(pos);
  Segmentations.append(actor);
}

//----------------------------------------------------------------------------
void vtkPVVolumeView::AddRepresentationInternal(vtkDataRepresentation* rep)
{
//     qDebug() << "ADDING REPRESENTATION";
  if (m_pendingActor)
  {
    m_reps[rep] = m_pendingActor;
    m_pendingActor = NULL;
  }
  vtkPVRenderView::AddRepresentationInternal(rep);
}

//----------------------------------------------------------------------------
void vtkPVVolumeView::RemoveRepresentationInternal(vtkDataRepresentation* rep)
{
  //   qDebug() << "REMOVING REPRESENTATION";
  VolumeActor *actor = m_reps.value(rep, NULL);
  if (actor)
  {
    RemoveActor(actor);
    m_reps.remove(rep);
  }
  vtkPVRenderView::RemoveRepresentationInternal(rep);
}

//----------------------------------------------------------------------------
void vtkPVVolumeView::Initialize ( unsigned int id )
{
  vtkPVRenderView::Initialize ( id );
  this->RenderView->GetRenderer()->UseDepthPeelingOff();
//   this->OverviewRenderer->UseDepthPeelingOff();

  this->RenderView->GetRenderer()->GetActiveCamera()->ParallelProjectionOn();
//   this->OverviewRenderer->GetActiveCamera()->ParallelProjectionOn();
}

//----------------------------------------------------------------------------
void vtkPVVolumeView::ResetCamera()
{
  vtkCamera *camera = RenderView->GetRenderer()->GetActiveCamera();
  camera->SetPosition (Crosshair[0], Crosshair[1], Crosshair[2]-1);
  camera->SetFocalPoint(Crosshair[0], Crosshair[1], Crosshair[2]);
  camera->SetRoll(180);
  vtkPVRenderView::ResetCamera();
//   OverviewRenderer->ResetCamera ( this->LastComputedBounds );
}

//----------------------------------------------------------------------------
void vtkPVVolumeView::ResetCamera ( double bounds[6] )
{
  vtkCamera *camera = RenderView->GetRenderer()->GetActiveCamera();
  camera->SetPosition (Crosshair[0], Crosshair[1], Crosshair[2]-1);
  camera->SetFocalPoint(Crosshair[0], Crosshair[1], Crosshair[2]);
  camera->SetRoll(180);
  vtkPVRenderView::ResetCamera ( bounds );
//   OverviewRenderer->ResetCamera ( bounds );
}

//----------------------------------------------------------------------------
void vtkPVVolumeView::ResetCameraClippingRange()
{
  vtkPVRenderView::ResetCameraClippingRange();
//   OverviewRenderer->ResetCameraClippingRange ( this->LastComputedBounds );
//   SetCenter ( Center );

  updateRuler();
}

//----------------------------------------------------------------------------
void vtkPVVolumeView::SetOrientationAxesVisibility(bool)
{
  vtkPVRenderView::SetOrientationAxesVisibility(false);
}

//----------------------------------------------------------------------------
void vtkPVVolumeView::SetCenterAxesVisibility(bool)
{
  vtkPVRenderView::SetCenterAxesVisibility(false);
}


//----------------------------------------------------------------------------
void vtkPVVolumeView::SetBackground ( double r, double g, double b )
{
  vtkPVRenderView::SetBackground ( r,g,b );
//   OverviewRenderer->SetBackground ( r,g,b );
}

//----------------------------------------------------------------------------
void vtkPVVolumeView::SetCrosshair(double x, double y, double z)
{
//   qDebug() << "vtkPVVolumeView setting Center on " << x << y << z;
  bool crossHairChanged;

  crossHairChanged = Crosshair[0] != x || Crosshair[1] != y || Crosshair[2] != z;

  Crosshair[0] = x;
  Crosshair[1] = y;
  Crosshair[2] = z;

  AxialMatrix->SetElement(2, 3, Crosshair[2]);
  SagittalMatrix->SetElement(0, 3, Crosshair[0]);
  CoronalMatrix->SetElement(1, 3, Crosshair[1]);
}

//----------------------------------------------------------------------------
void vtkPVVolumeView::SetCrosshair(double center[3])
{
  //   qDebug() << "Setting Center3" << center[0] << center[1] << center[2];
  SetCrosshair(center[0], center[1], center[2]);
}

//----------------------------------------------------------------------------
void vtkPVVolumeView::SetFocus(double x, double y, double z)
{
  vtkCamera *camera = RenderView->GetRenderer()->GetActiveCamera();
//   camera->SetPosition (Center[0], Center[1], Center[2]-1);
  camera->SetFocalPoint(x, y, z);
  camera->SetRoll(180);
}

//----------------------------------------------------------------------------
void vtkPVVolumeView::SetFocus(double center[3])
{
  //   qDebug() << "Setting Center3" << center[0] << center[1] << center[2];
  SetFocus(center[0], center[1], center[2]);
}


//----------------------------------------------------------------------------
void vtkPVVolumeView::SetHCrossLineColor(double r, double g, double b)
{
//   HCrossLine->GetProperty()->SetColor(r, g, b);
//   HCrossLineColor[0] = r;
//   HCrossLineColor[1] = g;
//   HCrossLineColor[2] = b;
}

//----------------------------------------------------------------------------
void vtkPVVolumeView::SetHCrossLineColor(double color[3])
{
//   SetHCrossLineColor(color[0], color[1], color[2]);
}

//----------------------------------------------------------------------------
void vtkPVVolumeView::SetVCrossLineColor ( double r, double g, double b )
{
//   VCrossLine->GetProperty()->SetColor ( r,g,b );
//   VCrossLineColor[0] = r;
//   VCrossLineColor[1] = g;
//   VCrossLineColor[2] = b;
}

//----------------------------------------------------------------------------
void vtkPVVolumeView::SetVCrossLineColor ( double color[3] )
{
//   SetVCrossLineColor ( color[0], color[1], color[2] );
}

//----------------------------------------------------------------------------
void vtkPVVolumeView::SetShowSegmentations ( bool visible )
{
  //   qDebug() << "vtkPVVolumeView segmentation's visibility = " << value;
  VolumeActor *seg;
  foreach (seg, Segmentations)
  {
    seg->prop->SetVisibility(visible && seg->visible);
  }
  ShowSegmentations = visible;
}

//----------------------------------------------------------------------------
void vtkPVVolumeView::SetShowRuler ( bool visible )
{
  //   qDebug() << "vtkPVVolumeView segmentation's visibility = " << value;
//   Ruler->SetVisibility ( visible );
//   ShowRuler = visible;
}

//----------------------------------------------------------------------------
void vtkPVVolumeView::SetRulerColor ( double r, double g, double b )
{
  return;
  Ruler->GetProperty()->SetColor ( r,g,b );
  Ruler->GetLabelTextProperty()->SetColor ( r,g,b );
  Ruler->GetTitleTextProperty()->SetColor ( r,g,b );
  RulerColor[0] = r;
  RulerColor[1] = g;
  RulerColor[2] = b;
}

//----------------------------------------------------------------------------
void vtkPVVolumeView::SetRulerColor ( double color[3] )
{
  SetRulerColor ( color[0], color[1], color[2] );
}

//----------------------------------------------------------------------------
void vtkPVVolumeView::updateRuler()
{
  return;
  if ( !ShowRuler )
    return;

  double *value;
  double left, right;

  double wPad = 60, hPad  = 100;

  int *ws = RenderView->GetRenderWindow()->GetSize();

  bool canDisplayRuler = ws[0] > RulerSize[0] + 2*wPad;
  Ruler->SetVisibility ( canDisplayRuler );
  if ( !canDisplayRuler )
    return;

  Ruler->SetPoint1(wPad/ws[0], hPad/ws[1]);
  Ruler->SetPoint2((wPad+RulerSize[0])/ws[0], hPad/ws[1]);

  vtkSmartPointer<vtkCoordinate> coords = vtkSmartPointer<vtkCoordinate>::New();

  coords->SetViewport(GetRenderer());
  coords->SetCoordinateSystemToNormalizedViewport();

  coords->SetValue ( 0,0 );
  value = coords->GetComputedWorldValue ( GetRenderer() );
  left = value[0];
  //       qDebug() << "UR" << value[0] << value[1] << value[2];
  coords->SetValue ( 1,0 );
  value = coords->GetComputedWorldValue ( GetRenderer() );
  right = value[0];
  //       qDebug() << "LL" << value[0] << value[1] << value[2];

  const double RULERWIDTHRATIO = fabs(Ruler->GetPoint1()[0] - Ruler->GetPoint2()[0]);
  double maxRange = fabs ( right-left ) *RULERWIDTHRATIO;
  //       qDebug() << "Max Range" << maxRange;

  std::string units[4] = {"nm","um", "mm", "m"};
  int unit = 0;
  while ( maxRange > 1000 )
  {
    maxRange /= 1000;
    unit++;
  }

  if ( maxRange < 0 && 0 == unit )
    maxRange = 0;
  Ruler->SetRange ( 0, maxRange );
  Ruler->SetTitle ( units[unit].c_str() );
}