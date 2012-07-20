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
*/


#include "MeshRenderer.h"
#include <vtkRenderWindow.h>
#include <model/Segmentation.h>
#include <ColorEngine.h>
#include <EspinaCore.h>
#include <vtkSmartPointer.h>
#include <vtkDiscreteMarchingCubes.h>
#include <vtkWindowedSincPolyDataFilter.h>
#include <vtkDecimatePro.h>
#include <vtkPolyDataMapper.h>
#include <vtkPolyDataNormals.h>
#include <vtkActor.h>
#include <vtkProperty.h>
#include <vtkImageConstantPad.h>
#include <vtkMath.h>

//-----------------------------------------------------------------------------
bool MeshRenderer::addItem(ModelItem* item)
{
  if (ModelItem::SEGMENTATION != item->type())
    return false;

  Segmentation *seg = dynamic_cast<Segmentation *>(item);

  // duplicated item? addItem again
  if (m_segmentations.contains(item))
    {

      m_renderer->RemoveActor(this->m_segmentations[seg].actor);
      m_segmentations[seg].actor->Delete();
      m_segmentations.remove(item);
    }

  ColorEngine *engine = EspinaCore::instance()->colorSettings().engine();
  QColor color = engine->color(seg);

  // segmentation image need to be padded to avoid segmentation voxels from touching the edges of the
  // image (and create morphologicaly correct actors)
  vtkSmartPointer<vtkImageConstantPad> padfilter = vtkSmartPointer<vtkImageConstantPad>::New();
  int extent[6];
  VolumeExtent(seg->volume(), extent);
  padfilter->SetInputConnection(seg->image());
  padfilter->SetOutputWholeExtent(extent[0]-1, extent[1]+1, extent[2]-1, extent[3]+1, extent[4]-1, extent[5]+1);
  padfilter->SetConstant(0);

  vtkSmartPointer<vtkDiscreteMarchingCubes> march = vtkSmartPointer<vtkDiscreteMarchingCubes>::New();
  march->ReleaseDataFlagOn();
  march->SetNumberOfContours(1);
  march->GenerateValues(1, 255, 255);
  march->ComputeScalarsOff();
  march->ComputeNormalsOff();
  march->ComputeGradientsOff();
  march->SetInputConnection(padfilter->GetOutputPort());

  vtkSmartPointer<vtkDecimatePro> decimate = vtkSmartPointer<vtkDecimatePro>::New();
  decimate->ReleaseDataFlagOn();
  decimate->SetGlobalWarningDisplay(false);
  decimate->SetTargetReduction(0.95);
  decimate->PreserveTopologyOn();
  decimate->SplittingOff();
  decimate->SetInputConnection(march->GetOutputPort());

  vtkSmartPointer<vtkWindowedSincPolyDataFilter> smoother = vtkSmartPointer<vtkWindowedSincPolyDataFilter>::New();
  smoother->ReleaseDataFlagOn();
  smoother->SetGlobalWarningDisplay(false);
  smoother->BoundarySmoothingOn();
  smoother->FeatureEdgeSmoothingOn();
  smoother->SetNumberOfIterations(15);
  smoother->SetFeatureAngle(120);
  smoother->SetEdgeAngle(90);
  smoother->SetInputConnection(decimate->GetOutputPort());

  vtkSmartPointer<vtkPolyDataNormals> normals = vtkSmartPointer<vtkPolyDataNormals>::New();
  normals->ReleaseDataFlagOn();
  normals->SetFeatureAngle(120);
  normals->SetInputConnection(smoother->GetOutputPort());

  vtkSmartPointer<vtkPolyDataMapper> isoMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  isoMapper->ReleaseDataFlagOn();
  isoMapper->ImmediateModeRenderingOn();
  isoMapper->ScalarVisibilityOff();
  isoMapper->SetInputConnection(normals->GetOutputPort());

  vtkActor *actor = vtkActor::New();
  actor->SetMapper(isoMapper);
  actor->GetProperty()->SetColor(color.redF(), color.greenF(), color.blueF());
  actor->GetProperty()->SetSpecular(0.2);
  actor->GetProperty()->SetOpacity(1);

  m_segmentations[seg].selected = !seg->isSelected();
  m_segmentations[seg].visible = seg->visible();
  m_segmentations[seg].color = engine->color(seg);
  m_segmentations[seg].actor = actor;

  if (this->m_enable)
    m_renderer->AddActor(actor);

  m_renderer->ResetCamera();
  m_renderer->GetRenderWindow()->Render();
  updateItem(seg);

  return true;
}

//-----------------------------------------------------------------------------
bool MeshRenderer::updateItem(ModelItem* item)
{
   if (ModelItem::SEGMENTATION != item->type())
     return false;

   bool updated = false;
   Segmentation *seg = dynamic_cast<Segmentation *>(item);
   Q_ASSERT(m_segmentations.contains(seg));
   Representation &rep = m_segmentations[seg];

   if (seg->isSelected() != rep.selected
     || seg->visible() != rep.visible
     || seg->data(Qt::DecorationRole).value<QColor>() != rep.color)
   {
     rep.selected = seg->isSelected();
     rep.visible  = seg->visible();
     rep.color = seg->data(Qt::DecorationRole).value<QColor>();

     double rgb[3] = { rep.color.redF(), rep.color.greenF(), rep.color.blueF() };
     double hsv[3] = { 0.0, 0.0, 0.0 };
     vtkMath::RGBToHSV(rgb,hsv);
     hsv[2] = (rep.selected ? 1.0 : 0.6);
     vtkMath::HSVToRGB(hsv, rgb);

     rep.actor->GetProperty()->SetColor(rgb[0], rgb[1], rgb[2]);
     rep.actor->GetProperty()->Modified();

     rep.actor->SetVisibility(rep.visible);
     rep.actor->Modified();

     m_renderer->GetRenderWindow()->Render();
     updated = true;
   }

   return updated;
}

//-----------------------------------------------------------------------------
bool MeshRenderer::removeItem(ModelItem* item)
{
   if (ModelItem::SEGMENTATION != item->type())
     return false;

   Segmentation *seg = dynamic_cast<Segmentation *>(item);
   Q_ASSERT(m_segmentations.contains(seg));

   if (this->m_enable)
     m_renderer->RemoveActor(m_segmentations[seg].actor);

   m_renderer->GetRenderWindow()->Render();

   m_segmentations[seg].actor->Delete();
   m_segmentations.remove(seg);

   return true;
}

//-----------------------------------------------------------------------------
void MeshRenderer::hide()
{
  if (!this->m_enable)
    return;

   foreach(Representation rep, m_segmentations)
     m_renderer->RemoveActor(rep.actor);

   m_renderer->GetRenderWindow()->Render();
   emit renderRequested();
}

//-----------------------------------------------------------------------------
void MeshRenderer::show()
{
  if (this->m_enable)
    return;

   foreach(Representation rep, m_segmentations)
     m_renderer->AddActor(rep.actor);

   m_renderer->GetRenderWindow()->Render();
   emit renderRequested();
}
