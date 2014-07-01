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


#include "CountingFrames/OrtogonalCountingFrame.h"

#include "Extensions/CountingFrameExtension.h"
#include "CountingFrames/vtkCountingFrameSliceWidget.h"
#include "CountingFrames/vtkCountingFrame3DWidget.h"

#include <Core/Analysis/Channel.h>
#include <GUI/View/View2D.h>
#include <GUI/View/View3D.h>
#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkPolyData.h>
#include <vtkRenderWindow.h>
#include <vtkSmartPointer.h>

using namespace EspINA;
using namespace EspINA::CF;

//-----------------------------------------------------------------------------
OrtogonalCountingFrame::OrtogonalCountingFrame(CountingFrameExtension *channelExt,
                                               const Bounds &bounds,
                                               Nm inclusion[3],
                                               Nm exclusion[3],
                                               SchedulerSPtr scheduler)
: CountingFrame(channelExt, inclusion, exclusion, scheduler)
, m_bounds(bounds)
{
  updateCountingFrameImplementation();
}


//-----------------------------------------------------------------------------
OrtogonalCountingFrame::~OrtogonalCountingFrame()
{
  for(auto view: m_widgets2D.keys())
    unregisterView(view);

  for(auto view: m_widgets3D.keys())
    unregisterView(view);
}

//-----------------------------------------------------------------------------
void OrtogonalCountingFrame::registerView(RenderView *view)
{
  View3D *view3d = dynamic_cast<View3D *>(view);
  if(view3d)
  {
    if(m_widgets3D.keys().contains(view3d))
      return;

    auto wa = CountingFrame3DWidgetAdapter::New();
    wa->SetCountingFrame(m_countingFrame, m_inclusion, m_exclusion);
    wa->SetCurrentRenderer(view3d->mainRenderer());
    wa->SetInteractor(view3d->renderWindow()->GetInteractor());
    wa->SetEnabled(true);

    m_widgets3D[view3d] = wa;
  }
  else
  {
    View2D *view2d = dynamic_cast<View2D *>(view);
    if(view2d)
    {
      if(m_widgets2D.keys().contains(view2d))
        return;

      auto wa = CountingFrame2DWidgetAdapter::New();
      wa->AddObserver(vtkCommand::EndInteractionEvent, m_command);
      wa->SetPlane(view2d->plane());
      wa->SetSlicingStep(m_extension->extendedItem()->output()->spacing());
      wa->SetCountingFrame(m_channelEdges, m_inclusion, m_exclusion);
      wa->SetSlice(view2d->crosshairPoint()[normalCoordinateIndex(view2d->plane())]);
      wa->SetCurrentRenderer(view2d->mainRenderer());
      wa->SetInteractor(view->mainRenderer()->GetRenderWindow()->GetInteractor());
      wa->SetEnabled(true);

      m_widgets2D[view2d] = wa;

      connect(view2d, SIGNAL(sliceChanged(Plane, Nm)), this, SLOT(sliceChanged(Plane, Nm)), Qt::QueuedConnection);
    }
  }
}

//-----------------------------------------------------------------------------
void OrtogonalCountingFrame::unregisterView(RenderView *view)
{
  View3D *view3d = dynamic_cast<View3D *>(view);
  if(view3d)
  {
    if(!m_widgets3D.keys().contains(view3d))
      return;

    m_widgets3D[view3d]->SetEnabled(false);
    view3d->mainRenderer()->RemoveActor(m_widgets3D[view3d]->GetRepresentation());
    m_widgets3D[view3d]->SetCurrentRenderer(nullptr);
    m_widgets3D[view3d]->SetInteractor(nullptr);
    m_widgets3D[view3d]->Delete();

    m_widgets3D.remove(view3d);
  }
  else
  {
    View2D *view2d = dynamic_cast<View2D *>(view);
    if(view2d)
    {
      if(!m_widgets2D.keys().contains(view2d))
        return;

      m_widgets2D[view2d]->SetEnabled(false);
      view2d->mainRenderer()->RemoveActor(m_widgets2D[view2d]->GetRepresentation());
      m_widgets2D[view2d]->RemoveObserver(m_command);
      m_widgets2D[view2d]->SetCurrentRenderer(nullptr);
      m_widgets2D[view2d]->SetInteractor(nullptr);
      m_widgets2D[view2d]->Delete();

      m_widgets2D.remove(view2d);

      disconnect(view2d, SIGNAL(sliceChanged(Plane, Nm)), this, SLOT(sliceChanged(Plane, Nm)));
    }
  }
}

//-----------------------------------------------------------------------------
void OrtogonalCountingFrame::updateCountingFrameImplementation()
{
  Nm Left   = m_bounds[0] + m_inclusion[0];
  Nm Top    = m_bounds[2] + m_inclusion[1];
  Nm Front  = m_bounds[4] + m_inclusion[2];
  Nm Right  = m_bounds[1] - m_exclusion[0];
  Nm Bottom = m_bounds[3] - m_exclusion[1];
  Nm Back   = m_bounds[5] - m_exclusion[2];

  m_countingFrame = createRectangularRegion(Left, Top, Front,
                                            Right, Bottom, Back);

  m_channelEdges = createRectangularRegion(m_bounds[0], m_bounds[2], m_bounds[4],
                                             m_bounds[1], m_bounds[3], m_bounds[5]);

  auto channel = m_extension->extendedItem();
  auto spacing = channel->output()->spacing();

  m_totalVolume = equivalentVolume(m_bounds);

  // Extract bounds corresponding to excluded voxels
  Bounds inclusionBounds{Left, Right-spacing[0], Top, Bottom-spacing[1], Front, Back-spacing[2]};
  m_inclusionVolume = equivalentVolume(inclusionBounds);
}

//-----------------------------------------------------------------------------
vtkSmartPointer<vtkPolyData> OrtogonalCountingFrame::createRectangularRegion(Nm left,  Nm top,    Nm front,
                                                                               Nm right, Nm bottom, Nm back)
{
  vtkSmartPointer<vtkPolyData>  region   = vtkSmartPointer<vtkPolyData>::New();
  vtkSmartPointer<vtkPoints>    vertex   = vtkSmartPointer<vtkPoints>::New();
  vtkSmartPointer<vtkCellArray> faces    = vtkSmartPointer<vtkCellArray>::New();
  vtkSmartPointer<vtkIntArray>  faceData = vtkSmartPointer<vtkIntArray>::New();

  vtkIdType frontFace[4], leftFace[4] , topFace[4];
  vtkIdType backFace[4] , rightFace[4], bottomFace[4];

    // Front Inclusion Face
  frontFace[0] = vertex->InsertNextPoint(left,  bottom, front );
  frontFace[1] = vertex->InsertNextPoint(left,  top,    front );
  frontFace[2] = vertex->InsertNextPoint(right, top,    front );
  frontFace[3] = vertex->InsertNextPoint(right, bottom, front );
  faces->InsertNextCell(4, frontFace);
  faceData->InsertNextValue(INCLUSION_FACE);

  // Back Exclusion Face
  backFace[0] = vertex->InsertNextPoint(left,  bottom, back);
  backFace[1] = vertex->InsertNextPoint(left,  top,    back);
  backFace[2] = vertex->InsertNextPoint(right, top,    back);
  backFace[3] = vertex->InsertNextPoint(right, bottom, back);
  faces->InsertNextCell(4, backFace);
  faceData->InsertNextValue(EXCLUSION_FACE);

  // Left Inclusion Face
  leftFace[0] = frontFace[0];
  leftFace[1] = frontFace[1];
  leftFace[2] = backFace[1];
  leftFace[3] = backFace[0];
  faces->InsertNextCell(4, leftFace);
  faceData->InsertNextValue(INCLUSION_FACE);

  // Right Exclusion Face
  rightFace[0] = frontFace[2];
  rightFace[1] = frontFace[3];
  rightFace[2] = backFace[3];
  rightFace[3] = backFace[2];
  faces->InsertNextCell(4, rightFace);
  faceData->InsertNextValue(EXCLUSION_FACE);

  // Top Inclusion Face
  topFace[0] = frontFace[1];
  topFace[1] = frontFace[2];
  topFace[2] = backFace[2];
  topFace[3] = backFace[1];
  faces->InsertNextCell(4, topFace);
  faceData->InsertNextValue(INCLUSION_FACE);

  // Bottom Exclusion Face
  bottomFace[0] = frontFace[3];
  bottomFace[1] = frontFace[0];
  bottomFace[2] = backFace[0];
  bottomFace[3] = backFace[3];
  faces->InsertNextCell(4, bottomFace);
  faceData->InsertNextValue(EXCLUSION_FACE);

  region->SetPoints(vertex);
  region->SetPolys(faces);
  vtkCellData *data = region->GetCellData();
  data->SetScalars(faceData);
  data->GetScalars()->SetName("Type");

  return region;
}
