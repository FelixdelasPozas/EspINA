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


#include "VolumetricRenderer.h"
#include <vtkRenderWindow.h>
#include <model/Segmentation.h>
#include <ColorEngine.h>
#include <EspinaCore.h>
#include <vtkVolumeRayCastMapper.h>
#include <vtkVolumeRayCastCompositeFunction.h>
#include <vtkColorTransferFunction.h>
#include <vtkPiecewiseFunction.h>
#include <vtkColorTransferFunction.h>
#include <vtkVolumeProperty.h>
#include <vtkSmartPointer.h>
#include <vtkMath.h>

//-----------------------------------------------------------------------------
bool VolumetricRenderer::addItem(ModelItem* item)
{
  if (ModelItem::SEGMENTATION != item->type())
    return false;

  Segmentation *seg = dynamic_cast<Segmentation *>(item);

  // duplicated item? addItem again
  if (m_segmentations.contains(item))
    {
      if (this->m_segmentations[seg].visible)
        m_renderer->RemoveVolume(this->m_segmentations[seg].volume);
      m_segmentations[seg].volume->Delete();
      m_segmentations.remove(item);
    }

  ColorEngine *engine = EspinaCore::instance()->colorSettings().engine();
  QColor color = engine->color(seg);

  vtkSmartPointer<vtkVolumeRayCastMapper> mapper = vtkVolumeRayCastMapper::New();
  mapper->ReleaseDataFlagOn();
  mapper->SetBlendModeToComposite();
  mapper->SetInputConnection(seg->image());

  vtkSmartPointer<vtkVolumeRayCastCompositeFunction> composite = vtkSmartPointer<vtkVolumeRayCastCompositeFunction>::New();
  mapper->SetVolumeRayCastFunction(composite);

  vtkSmartPointer<vtkColorTransferFunction> colorfunction = vtkSmartPointer<vtkColorTransferFunction>::New();
  colorfunction->AllowDuplicateScalarsOff();
  colorfunction->AddRGBPoint(255, color.redF(), color.greenF(), color.blueF());

  vtkSmartPointer<vtkPiecewiseFunction> piecewise = vtkSmartPointer<vtkPiecewiseFunction>::New();
  piecewise->AddPoint(0, 0.0);
  piecewise->AddPoint(255, 1.0);

  vtkSmartPointer<vtkVolumeProperty> property = vtkSmartPointer<vtkVolumeProperty>::New();
  property->SetColor(colorfunction);
  property->SetScalarOpacity(piecewise);
  property->DisableGradientOpacityOff();
  property->SetSpecular(0.5);
  property->ShadeOn();
  property->SetInterpolationTypeToLinear();

  vtkVolume *volume = vtkVolume::New();
  volume->SetMapper(mapper);
  volume->SetProperty(property);

  m_segmentations[seg].selected = !seg->isSelected();
  m_segmentations[seg].visible = seg->visible();
  m_segmentations[seg].color = engine->color(seg);
  m_segmentations[seg].volume = volume;

  if (this->m_enable)
    m_renderer->AddVolume(volume);

  m_renderer->ResetCamera();
  m_renderer->GetRenderWindow()->Render();
  updateItem(seg);

  return true;
}

//-----------------------------------------------------------------------------
bool VolumetricRenderer::updateItem(ModelItem* item)
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
     rep.color = seg->data(Qt::DecorationRole).value<QColor>();

     vtkVolumeProperty *property = rep.volume->GetProperty();
     vtkColorTransferFunction *color = property->GetRGBTransferFunction();
     double rgb[3] = { rep.color.redF(), rep.color.greenF(), rep.color.blueF() };
     double hsv[3] = { 0.0, 0.0, 0.0 };
     vtkMath::RGBToHSV(rgb,hsv);
     color->AddHSVPoint(255, hsv[0], hsv[1], rep.selected ? 1.0 : 0.6);
     color->Modified();

     if (rep.visible != seg->visible())
     {
         if (seg->visible())
         {
           if (m_enable)
             m_renderer->AddVolume(rep.volume);
         }
         else
           m_renderer->RemoveVolume(rep.volume);
     }
     rep.visible = seg->visible();

     m_renderer->GetRenderWindow()->Render();
     updated = true;
   }

   return updated;
}

//-----------------------------------------------------------------------------
bool VolumetricRenderer::removeItem(ModelItem* item)
{
   if (ModelItem::SEGMENTATION != item->type())
     return false;

   Segmentation *seg = dynamic_cast<Segmentation *>(item);
   Q_ASSERT(m_segmentations.contains(seg));

   if (this->m_enable && m_segmentations[seg].visible)
     m_renderer->RemoveVolume(m_segmentations[seg].volume);

   m_renderer->GetRenderWindow()->Render();

   m_segmentations[seg].volume->Delete();
   m_segmentations.remove(seg);

   return true;
}

//-----------------------------------------------------------------------------
void VolumetricRenderer::hide()
{
  if (!this->m_enable)
    return;

   foreach(Representation rep, m_segmentations)
     if (rep.visible)
       m_renderer->RemoveVolume(rep.volume);

   m_renderer->GetRenderWindow()->Render();
   emit renderRequested();
}

//-----------------------------------------------------------------------------
void VolumetricRenderer::show()
{
  if (this->m_enable)
    return;

   foreach(Representation rep, m_segmentations)
     if (rep.visible)
       m_renderer->AddVolume(rep.volume);

   m_renderer->GetRenderWindow()->Render();
   emit renderRequested();
}
