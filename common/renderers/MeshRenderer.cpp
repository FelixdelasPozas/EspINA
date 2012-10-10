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
#include <vtkSmartPointer.h>
#include <vtkDiscreteMarchingCubes.h>
#include <vtkWindowedSincPolyDataFilter.h>
#include <vtkDecimatePro.h>
#include <vtkPolyDataMapper.h>
#include <vtkPolyDataNormals.h>
#include <vtkActor.h>
#include <vtkProperty.h>
#include <vtkImageConstantPad.h>
#include <vtkAlgorithm.h>
#include <vtkMath.h>
#include <QApplication>

//-----------------------------------------------------------------------------
bool MeshRenderer::addItem(ModelItem* item)
{
  if (ModelItem::SEGMENTATION != item->type())
    return false;

  Segmentation *seg = dynamic_cast<Segmentation *>(item);

  // duplicated item? addItem again
  if (m_segmentations.contains(item))
    {
      if (m_enable)
        m_renderer->RemoveActor(this->m_segmentations[seg].actor);
      m_segmentations[seg].actor->Delete();
      m_segmentations.remove(item);
    }

  /*TODO 2012-10-05
  ColorEngine *engine = EspinaCore::instance()->colorSettings().engine();
  QColor color = engine->color(seg);
  */
  QColor color(Qt::blue);

  vtkSmartPointer<vtkDecimatePro> decimate = vtkSmartPointer<vtkDecimatePro>::New();
  decimate->ReleaseDataFlagOn();
  decimate->SetGlobalWarningDisplay(false);
  decimate->SetTargetReduction(0.95);
  decimate->PreserveTopologyOn();
  decimate->SplittingOff();
  decimate->SetInputConnection(seg->mesh());

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
  double rgb[3] = { color.redF(), color.greenF(), color.blueF() };
  double hsv[3] = { 0.0, 0.0, 0.0 };
  vtkMath::RGBToHSV(rgb,hsv);
  hsv[2] = (seg->isSelected() ? 1.0 : 0.6);
  vtkMath::HSVToRGB(hsv, rgb);
  actor->GetProperty()->SetColor(rgb[0], rgb[1], rgb[2]);
  actor->GetProperty()->SetSpecular(0.2);
  actor->GetProperty()->SetOpacity(1);

  m_segmentations[seg].selected = seg->isSelected();
  //TODO 2012-10-05 m_segmentations[seg].color = engine->color(seg);
  m_segmentations[seg].actor = actor;
  m_segmentations[seg].visible = false;

  int extent[6];
  vtkImageData *image = vtkImageData::SafeDownCast(seg->vtkVolume()->GetProducer()->GetOutputDataObject(0));
  image->GetExtent(extent);
  memcpy(m_segmentations[seg].extent, extent, 6*sizeof(int));

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

   // the representation must be updated, whether displayed or not
   if (seg->isSelected() != rep.selected
     || seg->data(Qt::DecorationRole).value<QColor>() != rep.color)
   {
     rep.selected = seg->isSelected();
     rep.color = seg->data(Qt::DecorationRole).value<QColor>();

     double rgb[3] = { rep.color.redF(), rep.color.greenF(), rep.color.blueF() };
     double hsv[3] = { 0.0, 0.0, 0.0 };
     vtkMath::RGBToHSV(rgb,hsv);
     hsv[2] = (rep.selected ? 1.0 : 0.6);
     vtkMath::HSVToRGB(hsv, rgb);
     rep.actor->GetProperty()->SetColor(rgb[0], rgb[1], rgb[2]);

     updated = true;
   }

   int extent[6];
   vtkImageData *image = vtkImageData::SafeDownCast(seg->vtkVolume()->GetProducer()->GetOutputDataObject(0));
   image->GetExtent(extent);
   if (memcmp(extent, rep.extent, 6*sizeof(int)) != 0)
   {
     memcpy(m_segmentations[seg].extent, extent, 6*sizeof(int));
     updated = true;
   }

   if (m_enable && seg->visible())
   {
     if (!rep.visible)
     {
       m_renderer->AddActor(rep.actor);
       rep.visible = true;
       updated = true;
     }
   }
   else if (rep.visible)
   {
     m_renderer->RemoveActor(rep.actor);
     rep.visible = false;
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

   if (m_segmentations[seg].visible)
     m_renderer->RemoveActor(m_segmentations[seg].actor);

   m_segmentations[seg].actor->Delete();
   m_segmentations.remove(seg);

   return true;
}

//-----------------------------------------------------------------------------
void MeshRenderer::hide()
{
  if (!this->m_enable)
    return;

  m_enable = false;
  QMap<ModelItem *, Representation>::iterator it;

  for (it = m_segmentations.begin(); it != m_segmentations.end(); it++)
    if ((*it).visible)
    {
      m_renderer->RemoveActor((*it).actor);
      (*it).visible = false;
    }

   emit renderRequested();
}

//-----------------------------------------------------------------------------
void MeshRenderer::show()
{
  if (this->m_enable)
    return;

  m_enable = true;
  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

  QMap<ModelItem *, Representation>::iterator it;

  for (it = m_segmentations.begin(); it != m_segmentations.end(); it++)
    if (!(*it).visible)
    {
      m_renderer->AddActor((*it).actor);
      (*it).visible = true;
    }

  emit renderRequested();
  QApplication::restoreOverrideCursor();
}

//-----------------------------------------------------------------------------
unsigned int MeshRenderer::getNumberOfvtkActors()
{
  unsigned int numActors = 0;

  QMap<ModelItem *, Representation>::iterator it;
  for (it = m_segmentations.begin(); it != m_segmentations.end(); it++)
    if ((*it).visible)
      numActors++;

  return numActors;
}
