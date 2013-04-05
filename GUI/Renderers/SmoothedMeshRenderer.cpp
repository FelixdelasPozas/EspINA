/*
 <one line to give the program's name and a brief idea of what it does.>
 Copyright (C) 2013 Félix de las Pozas Álvarez <felixdelaspozas@gmail.com>

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

// EspINA
#include "SmoothedMeshRenderer.h"
#include <Core/Model/ModelItem.h>
#include <Core/EspinaTypes.h>
#include <Core/Model/Segmentation.h>
#include <GUI/ViewManager.h>

// VTK
#include <vtkWindowedSincPolyDataFilter.h>
#include <vtkDecimatePro.h>
#include <vtkPolyDataMapper.h>
#include <vtkPolyDataNormals.h>
#include <vtkActor.h>
#include <vtkMath.h>
#include <vtkProperty.h>
#include <vtkPropPicker.h>

// Qt
#include <QApplication>

using namespace EspINA;

//-----------------------------------------------------------------------------
SmoothedMeshRenderer::SmoothedMeshRenderer(ViewManager *vm, QObject *parent)
: MeshRenderer(vm, parent)
{
}

//-----------------------------------------------------------------------------
bool SmoothedMeshRenderer::addItem(ModelItemPtr item)
{
  if (EspINA::SEGMENTATION != item->type())
    return false;

  if (!itemCanBeRendered(item))
    return false;

  SegmentationPtr seg = segmentationPtr(item);

  // duplicated item? addItem again
  if (m_segmentations.contains(item))
  {
    if (m_enable)
    {
      m_renderer->RemoveActor(this->m_segmentations[seg].actor);
      m_picker->DeletePickList(m_segmentations[seg].actor);
    }

    m_segmentations[seg].actor->Delete();
    m_segmentations[seg].actorPropertyBackup = NULL;
    m_segmentations.remove(item);

    m_decimate[seg] = NULL;
    m_decimate.remove(item);
  }
  
  QColor color = m_viewManager->color(seg);

  m_decimate[seg] = vtkSmartPointer<vtkDecimatePro>::New();
  m_decimate[seg]->ReleaseDataFlagOn();
  m_decimate[seg]->SetGlobalWarningDisplay(false);
  m_decimate[seg]->SetTargetReduction(0.95);
  m_decimate[seg]->PreserveTopologyOn();
  m_decimate[seg]->SplittingOff();
  m_decimate[seg]->SetInputConnection(seg->volume()->toMesh());

  vtkSmartPointer<vtkWindowedSincPolyDataFilter> smoother = vtkSmartPointer<vtkWindowedSincPolyDataFilter>::New();
  smoother->ReleaseDataFlagOn();
  smoother->SetGlobalWarningDisplay(false);
  smoother->BoundarySmoothingOn();
  smoother->FeatureEdgeSmoothingOn();
  smoother->SetNumberOfIterations(15);
  smoother->SetFeatureAngle(120);
  smoother->SetEdgeAngle(90);
  smoother->SetInputConnection(m_decimate[seg]->GetOutputPort());

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
  double rgb[3] = { color.redF(), color.greenF(), color.blueF() };
  double hsv[3] = { 0.0, 0.0, 0.0 };
  vtkMath::RGBToHSV(rgb, hsv);
  hsv[2] = (seg->isSelected() ? 1.0 : 0.6);
  vtkMath::HSVToRGB(hsv, rgb);
  actor->GetProperty()->SetColor(rgb[0], rgb[1], rgb[2]);
  actor->GetProperty()->SetSpecular(0.2);
  actor->GetProperty()->SetOpacity(1);

  m_segmentations[seg].selected = seg->isSelected();
  m_segmentations[seg].color = m_viewManager->color(seg);
  m_segmentations[seg].actor = actor;
  m_segmentations[seg].visible = false;
  m_segmentations[seg].overridden = seg->OverridesRendering();
  m_segmentations[seg].renderingType = seg->getHierarchyRenderingType();
  m_segmentations[seg].actorPropertyBackup = NULL;

  int extent[6];
  vtkImageData *image = vtkImageData::SafeDownCast(seg->volume()->toVTK()->GetProducer()->GetOutputDataObject(0));
  image->GetExtent(extent);
  memcpy(m_segmentations[seg].extent, extent, 6 * sizeof(int));

  if (m_enable)
  {
    m_segmentations[seg].visible = true;
    m_renderer->AddActor(actor);
    m_picker->AddPickList(actor);
  }

  if (m_segmentations[seg].overridden)
    createHierarchyProperties(seg);

  m_segmentations[seg].actor->Modified();

  connect(item, SIGNAL(modified(ModelItemPtr)), this, SLOT(updateItem(ModelItemPtr)));

  return true;
}

//-----------------------------------------------------------------------------
bool SmoothedMeshRenderer::updateItem(ModelItemPtr item, bool forced)
{
  if (!m_enable && !forced)
    return false;

  if (EspINA::SEGMENTATION != item->type())
    return false;

  bool updated = false;
  bool hierarchiesUpdated = false;
  SegmentationPtr seg = segmentationPtr(item);

  if (!m_segmentations.contains(seg))
  {
    if (itemCanBeRendered(item))
      return addItem(item);

    return false;
  }
  else
  {
    if (!itemCanBeRendered(item))
      return removeItem(item);
  }

  Representation &rep = m_segmentations[seg];
  if (seg->visible())
  {
    if (!rep.visible)
    {
      m_renderer->AddActor(rep.actor);
      m_picker->AddPickList(rep.actor);
      rep.visible = true;
      updated = true;
    }
  }
  else
  {
    // return avoiding updated in the VTK pipelines.
    if (rep.visible)
    {
      m_renderer->RemoveActor(rep.actor);
      m_picker->DeletePickList(rep.actor);
      rep.visible = false;
      return true;
    }
    return false;
  }

  // check if the beginning of the pipeline has changed
  if (m_decimate[seg]->GetInputConnection(0, 0) != seg->volume()->toMesh())
  {
    m_decimate[seg]->SetInputConnection(seg->volume()->toMesh());
    m_decimate[seg]->Update();

    updated = true;
  }

  // deal with hierarchies first
  vtkSmartPointer<vtkProperty> actorProperty = NULL;
  if (seg->OverridesRendering())
  {
    if (m_segmentations[seg].actorPropertyBackup == NULL)
    {
      createHierarchyProperties(seg);
      hierarchiesUpdated = true;
    }
    else
      hierarchiesUpdated = updateHierarchyProperties(seg);

    actorProperty = m_segmentations[seg].actorPropertyBackup;
  }
  else
  {
    if (m_segmentations[seg].overridden != seg->OverridesRendering())
      hierarchiesUpdated = updateHierarchyProperties(seg);

    actorProperty = m_segmentations[seg].actor->GetProperty();
  }

  int extent[6];
  vtkImageData *image = vtkImageData::SafeDownCast(seg->volume()->toVTK()->GetProducer()->GetOutputDataObject(0));
  image->GetExtent(extent);
  if (memcmp(extent, rep.extent, 6 * sizeof(int)) != 0)
  {
    memcpy(m_segmentations[seg].extent, extent, 6 * sizeof(int));
    updated = true;
  }

  if (seg->isSelected() != rep.selected || m_viewManager->color(seg) != rep.color)
  {
    rep.selected = seg->isSelected();
    rep.color = m_viewManager->color(seg);

    double rgb[3] = { rep.color.redF(), rep.color.greenF(), rep.color.blueF() };
    double hsv[3] = { 0.0, 0.0, 0.0 };
    vtkMath::RGBToHSV(rgb, hsv);
    hsv[2] = (rep.selected ? 1.0 : 0.6);
    vtkMath::HSVToRGB(hsv, rgb);
    rep.actor->GetProperty()->SetColor(rgb[0], rgb[1], rgb[2]);
    rep.actor->GetProperty()->Modified();

    updated = true;
  }

  return updated || hierarchiesUpdated;
}

//-----------------------------------------------------------------------------
bool SmoothedMeshRenderer::removeItem(ModelItemPtr item)
{
   if (EspINA::SEGMENTATION != item->type())
     return false;

   SegmentationPtr seg = segmentationPtr(item);
   if(!m_segmentations.contains(seg))
     return false;

   if (m_segmentations[seg].visible)
   {
     m_renderer->RemoveActor(m_segmentations[seg].actor);
     m_picker->DeletePickList(m_segmentations[seg].actor);
   }

   if (m_segmentations[seg].actorPropertyBackup)
     m_segmentations[seg].actorPropertyBackup = NULL;

   m_segmentations[seg].actor->Delete();
   m_segmentations.remove(seg);

   m_decimate[seg] = NULL;
   m_decimate.remove(seg);

   disconnect(item, SIGNAL(modified(ModelItemPtr)), this, SLOT(updateItem(ModelItemPtr)));

   return true;
}
