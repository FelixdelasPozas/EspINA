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

#include "GUI/ViewManager.h"
#include <Core/Model/Segmentation.h>
#include <Core/ColorEngines/IColorEngine.h>
#include <Core/Model/HierarchyItem.h>

#include <vtkRenderWindow.h>
#include <vtkVolumeRayCastMapper.h>
#include <vtkVolumeRayCastCompositeFunction.h>
#include <vtkColorTransferFunction.h>
#include <vtkPiecewiseFunction.h>
#include <vtkColorTransferFunction.h>
#include <vtkVolumeProperty.h>
#include <vtkSmartPointer.h>
#include <vtkMath.h>

#include <QApplication>

using namespace EspINA;

//-----------------------------------------------------------------------------
VolumetricRenderer::VolumetricRenderer(ViewManager* vm, QObject* parent)
: IRenderer(parent)
, m_viewManager(vm)
{
}

//-----------------------------------------------------------------------------
bool VolumetricRenderer::addItem(ModelItemPtr item)
{
  if (EspINA::SEGMENTATION != item->type())
    return false;

  if (!itemCanBeRendered(item))
    return false;

  SegmentationPtr seg = segmentationPtr(item);

  // duplicated item? addItem again
  if (m_segmentations.contains(item))
    {
      if (this->m_segmentations[seg].visible)
        m_renderer->RemoveVolume(this->m_segmentations[seg].volume);
      m_segmentations[seg].volume->Delete();
      if (m_segmentations[seg].actorPropertyBackup)
        m_segmentations[seg].actorPropertyBackup = NULL;
      m_segmentations.remove(item);
    }

  QColor color = m_viewManager->color(seg);

  vtkSmartPointer<vtkVolumeRayCastMapper> mapper = vtkVolumeRayCastMapper::New();
  mapper->ReleaseDataFlagOn();
  mapper->SetBlendModeToComposite();
  mapper->IntermixIntersectingGeometryOff();
  mapper->SetInputConnection(seg->volume()->toVTK());

  vtkSmartPointer<vtkVolumeRayCastCompositeFunction> composite = vtkSmartPointer<vtkVolumeRayCastCompositeFunction>::New();
  mapper->SetVolumeRayCastFunction(composite);

  vtkSmartPointer<vtkColorTransferFunction> colorfunction = vtkSmartPointer<vtkColorTransferFunction>::New();
  colorfunction->AllowDuplicateScalarsOff();
  double rgb[3] = { color.redF(), color.greenF(), color.blueF() };
  double hsv[3] = { 0.0, 0.0, 0.0 };
  vtkMath::RGBToHSV(rgb,hsv);
  colorfunction->AddHSVPoint(255, hsv[0], hsv[1], seg->isSelected() ? 1.0 : 0.6);

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

  m_segmentations[seg].selected = seg->isSelected();
  m_segmentations[seg].color = m_viewManager->color(seg);
  m_segmentations[seg].volume = volume;
  m_segmentations[seg].visible = false;
  m_segmentations[seg].overridden = seg->OverridesRendering();
  m_segmentations[seg].overridden = seg->getHierarchyRenderingType();
  m_segmentations[seg].actorPropertyBackup = NULL;

  if (m_enable)
  {
    m_segmentations[seg].visible = true;
    m_renderer->AddVolume(volume);
  }

  if (m_segmentations[seg].overridden)
    this->createHierarchyProperties(seg);

  m_segmentations[seg].volume->Modified();
  return true;
}

//-----------------------------------------------------------------------------
bool VolumetricRenderer::updateItem(ModelItemPtr item)
{
  if (EspINA::SEGMENTATION != item->type())
    return false;

  bool updated = false;
  bool hierarchiesUpdated = false;
  SegmentationPtr seg = segmentationPtr(item);
  if (!m_segmentations.contains(seg))
    return false;

  if (!itemCanBeRendered(item))
  {
    removeItem(item);
    return false;
  }

  Representation &rep = m_segmentations[seg];
  vtkSmartPointer<vtkVolumeProperty> volumeProperty = NULL;

  // deal with hierarchies first
  if (seg->OverridesRendering())
  {
    if (m_segmentations[seg].actorPropertyBackup == NULL)
    {
      createHierarchyProperties(seg);
      hierarchiesUpdated = true;
    }
    else
      hierarchiesUpdated = updateHierarchyProperties(seg);

    volumeProperty = m_segmentations[seg].actorPropertyBackup;
  }
  else
  {
    if (m_segmentations[seg].overridden != seg->OverridesRendering())
      hierarchiesUpdated = updateHierarchyProperties(seg);

    volumeProperty = m_segmentations[seg].volume->GetProperty();
  }

  if (m_enable && seg->visible())
  {
    if (!rep.visible)
    {
      m_renderer->AddVolume(rep.volume);
      rep.visible = true;
      updated = true;
    }
  }
  else
    if (rep.visible)
    {
      m_renderer->RemoveVolume(rep.volume);
      rep.visible = false;
      updated = true;
    }

  if (seg->isSelected() != rep.selected || m_viewManager->color(seg) != rep.color)
  {
    rep.selected = seg->isSelected();
    rep.color = m_viewManager->color(seg);

    vtkColorTransferFunction *color = volumeProperty->GetRGBTransferFunction();
    double rgb[3] = { rep.color.redF(), rep.color.greenF(), rep.color.blueF() };
    double hsv[3] = { 0.0, 0.0, 0.0 };
    vtkMath::RGBToHSV(rgb, hsv);
    color->AddHSVPoint(255, hsv[0], hsv[1], rep.selected ? 1.0 : 0.6);
    color->Modified();

    updated = true;
  }

  return updated || hierarchiesUpdated;
}

//-----------------------------------------------------------------------------
bool VolumetricRenderer::removeItem(ModelItemPtr item)
{
   if (EspINA::SEGMENTATION != item->type())
     return false;

   SegmentationPtr seg = segmentationPtr(item);
   Q_ASSERT(m_segmentations.contains(seg));

   if (m_enable && m_segmentations[seg].visible)
     m_renderer->RemoveVolume(m_segmentations[seg].volume);

   if (m_segmentations[seg].actorPropertyBackup)
     m_segmentations[seg].actorPropertyBackup = NULL;

   m_segmentations[seg].volume->Delete();
   m_segmentations.remove(seg);

   return true;
}

//-----------------------------------------------------------------------------
void VolumetricRenderer::hide()
{
  if (!m_enable)
    return;

  QMap<ModelItemPtr, Representation>::iterator it;

  for (it = m_segmentations.begin(); it != m_segmentations.end(); ++it)
    if ((*it).visible)
    {
      m_renderer->RemoveVolume((*it).volume);
      (*it).visible = false;
    }

  emit renderRequested();
}

//-----------------------------------------------------------------------------
void VolumetricRenderer::show()
{
  if (m_enable)
    return;

  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

  QMap<ModelItemPtr, Representation>::iterator it;

  for (it = m_segmentations.begin(); it != m_segmentations.end(); ++it)
    if(!(*it).visible)
    {
      m_renderer->AddVolume((*it).volume);
      (*it).visible = true;
    }

  emit renderRequested();
  QApplication::restoreOverrideCursor();
}

//-----------------------------------------------------------------------------
void VolumetricRenderer::createHierarchyProperties(SegmentationPtr seg)
{
  m_segmentations[seg].actorPropertyBackup = m_segmentations[seg].volume->GetProperty();
  m_segmentations[seg].overridden = true;

  QColor color = m_segmentations[seg].color;
  double rgb[3], hsv[3] = { 0.0, 0.0, 0.0 };

  vtkSmartPointer<vtkColorTransferFunction> hierarchyColor = vtkSmartPointer<vtkColorTransferFunction>::New();
  hierarchyColor->AllowDuplicateScalarsOff();
  rgb[0] = color.redF();
  rgb[1] = color.greenF();
  rgb[2] = color.blueF();
  vtkMath::RGBToHSV(rgb,hsv);
  hierarchyColor->AddHSVPoint(255, hsv[0], hsv[1], seg->isSelected() ? 1.0 : 0.6);

  vtkSmartPointer<vtkPiecewiseFunction> hierarchyPiecewise = vtkSmartPointer<vtkPiecewiseFunction>::New();
  hierarchyPiecewise->AddPoint(0, 0.0);
  switch(seg->getHierarchyRenderingType())
  {
    case HierarchyItem::Opaque:
      hierarchyPiecewise->AddPoint(255, 1.0);
      if (m_enable && !m_segmentations[seg].visible)
      {
        m_segmentations[seg].visible = true;
        m_renderer->AddVolume(m_segmentations[seg].volume);
      }
      break;
    case HierarchyItem::Translucent:
      hierarchyPiecewise->AddPoint(255, 0.3);
      if (m_enable && !m_segmentations[seg].visible)
      {
        m_segmentations[seg].visible = true;
        m_renderer->AddVolume(m_segmentations[seg].volume);
      }
      break;
    case HierarchyItem::Hidden:
      if (m_enable && m_segmentations[seg].visible)
      {
        m_segmentations[seg].visible = false;
        m_renderer->RemoveVolume(m_segmentations[seg].volume);
      }
      break;
    case HierarchyItem::Undefined:
      break;
    default:
      Q_ASSERT(false);
      break;
  }

  vtkSmartPointer<vtkVolumeProperty> hierarchyProperty = vtkSmartPointer<vtkVolumeProperty>::New();
  hierarchyProperty->SetColor(hierarchyColor);
  hierarchyProperty->SetScalarOpacity(hierarchyPiecewise);
  hierarchyProperty->DisableGradientOpacityOff();
  hierarchyProperty->SetSpecular(0.5);
  hierarchyProperty->ShadeOn();
  hierarchyProperty->SetInterpolationTypeToLinear();

  m_segmentations[seg].volume->SetProperty(hierarchyProperty);
}

//-----------------------------------------------------------------------------
bool VolumetricRenderer::updateHierarchyProperties(SegmentationPtr seg)
{
  Q_ASSERT(m_segmentations[seg].actorPropertyBackup != NULL);
  bool updated = false;

  vtkSmartPointer<vtkVolumeProperty> volumeProperty = NULL;
  if (seg->OverridesRendering() != m_segmentations[seg].overridden)
  {
    volumeProperty = m_segmentations[seg].volume->GetProperty();
    m_segmentations[seg].volume->SetProperty(m_segmentations[seg].actorPropertyBackup);
    m_segmentations[seg].actorPropertyBackup = volumeProperty;
    m_segmentations[seg].volume->Modified();
    m_segmentations[seg].overridden = seg->OverridesRendering();
    updated = true;
  }

  if (!seg->OverridesRendering())
    return true;

  volumeProperty = m_segmentations[seg].volume->GetProperty();
  if (m_segmentations[seg].color != m_viewManager->color(seg))
  {
    QColor color = m_viewManager->color(seg);
    vtkColorTransferFunction *hierarchyColor = volumeProperty->GetRGBTransferFunction();
    double rgb[3] = { color.redF(), color.greenF(), color.blueF() };
    double hsv[3] = { 0.0, 0.0, 0.0 };
    vtkMath::RGBToHSV(rgb, hsv);
    hierarchyColor->AddHSVPoint(255, hsv[0], hsv[1], m_segmentations[seg].selected ? 1.0 : 0.6);
    hierarchyColor->Modified();
    updated = true;
  }

  if (seg->getHierarchyRenderingType() != m_segmentations[seg].renderingType)
  {
    m_segmentations[seg].renderingType = seg->getHierarchyRenderingType();
    vtkPiecewiseFunction *hierarchyPiecewise = volumeProperty->GetGradientOpacity();
    switch (m_segmentations[seg].renderingType)
    {
      case HierarchyItem::Opaque:
        hierarchyPiecewise->AddPoint(255, 1.0);
        if (m_enable && !m_segmentations[seg].visible)
        {
          m_segmentations[seg].visible = true;
          m_renderer->AddVolume(m_segmentations[seg].volume);
        }
        break;
      case HierarchyItem::Translucent:
        hierarchyPiecewise->AddPoint(255, 0.3);
        if (m_enable && !m_segmentations[seg].visible)
        {
          m_segmentations[seg].visible = true;
          m_renderer->AddVolume(m_segmentations[seg].volume);
        }
        break;
      case HierarchyItem::Hidden:
        if (m_enable && m_segmentations[seg].visible)
        {
          m_segmentations[seg].visible = false;
          m_renderer->RemoveVolume(m_segmentations[seg].volume);
        }
        break;
      case HierarchyItem::Undefined:
        break;
      default:
        Q_ASSERT(false);
        break;
    }
    updated = true;
  }

  if (updated)
    m_segmentations[seg].volume->Modified();

  return updated;
}
