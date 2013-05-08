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
VolumetricRenderer::VolumetricRenderer(QObject* parent)
: IRenderer(parent)
, m_picker(vtkSmartPointer<vtkVolumePicker>::New())
{
  m_picker->PickFromListOn();
  m_picker->SetPickClippingPlanes(false);
  m_picker->SetPickCroppingPlanes(false);
  m_picker->SetPickTextureData(false);
  m_picker->SetTolerance(0);
}

//-----------------------------------------------------------------------------
//bool VolumetricRenderer::addItem(ModelItemPtr item)
//{
  /* FIXME
  if (EspINA::SEGMENTATION != item->type())
    return false;

  if (!itemCanBeRendered(item))
    return false;

  SegmentationPtr seg = segmentationPtr(item);

  // duplicated item? addItem again
  if (m_segmentations.contains(item))
    {
      if (this->m_segmentations[seg].visible)
      {
        m_renderer->RemoveVolume(m_segmentations[seg].volume);
        m_picker->DeletePickList(m_segmentations[seg].volume);
      }

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
    m_picker->AddPickList(m_segmentations[seg].volume);
  }

  if (m_segmentations[seg].overridden)
    this->createHierarchyProperties(seg);

  m_segmentations[seg].volume->Modified();
  return true;
  */
//  return false;
//}

//-----------------------------------------------------------------------------
//bool VolumetricRenderer::updateItem(ModelItemPtr item, bool forced)
//{
  /* FIXME
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

  // has the beginning of the pipeline changed?
  Representation &rep = m_segmentations[seg];
  if (rep.volume->GetMapper()->GetInputConnection(0,0) != seg->volume()->toVTK())
  {
    rep.volume->GetMapper()->SetInputConnection(seg->volume()->toVTK());
    rep.volume->Update();

    updated = true;
  }

  if (seg->visible())
  {
    if (!rep.visible)
    {
      rep.volume->Update();
      m_renderer->AddVolume(rep.volume);
      m_picker->AddPickList(rep.volume);
      rep.visible = true;
      updated = true;
    }
  }
  else
  {
    // return avoiding updates in the VTK pipelines;
    if (rep.visible)
    {
      m_renderer->RemoveVolume(rep.volume);
      m_picker->DeletePickList(rep.volume);
      rep.visible = false;
      return true;
    }
    return false;
  }

  // deal with hierarchies first
  vtkSmartPointer<vtkVolumeProperty> volumeProperty = NULL;
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
  */
//  return false;
//}

//-----------------------------------------------------------------------------
//bool VolumetricRenderer::removeItem(ModelItemPtr item)
//{
//   if (EspINA::SEGMENTATION != item->type())
//     return false;
//
//   SegmentationPtr seg = segmentationPtr(item);
//   if(!m_segmentations.contains(seg))
//     return false;
//
//   if (m_enable && m_segmentations[seg].visible)
//   {
//     m_renderer->RemoveVolume(m_segmentations[seg].volume);
//     m_picker->DeletePickList(m_segmentations[seg].volume);
//   }
//
//   if (m_segmentations[seg].actorPropertyBackup)
//     m_segmentations[seg].actorPropertyBackup = NULL;
//
//   m_segmentations[seg].volume->Delete();
//   m_segmentations.remove(seg);
//
//   return true;
//}

//-----------------------------------------------------------------------------
void VolumetricRenderer::hide()
{
    if (!m_enable)
      return;

    foreach(GraphicalRepresentationSPtr rep, m_representations)
      rep->setVisible(false);

    emit renderRequested();
}

//-----------------------------------------------------------------------------
void VolumetricRenderer::show()
{
  if (!m_enable)
    return;

  foreach(GraphicalRepresentationSPtr rep, m_representations)
    rep->setVisible(true);

  emit renderRequested();
}

//-----------------------------------------------------------------------------
GraphicalRepresentationSList VolumetricRenderer::pick(int x, int y, bool repeat)
{
  GraphicalRepresentationSList selection;
  QList<vtkVolume *> removedProps;

  if (m_renderer)
  {
    while (m_picker->Pick(x, y, 0, m_renderer))
    {
      vtkVolume *pickedProp = m_picker->GetVolume();
      Q_ASSERT(pickedProp);

      m_picker->GetPickList()->RemoveItem(pickedProp);
      removedProps << pickedProp;

      foreach(GraphicalRepresentationSPtr rep, m_representations)
        if (rep->hasActor(pickedProp))
        {
          selection << rep;
          break;
        }

      if (!repeat)
        break;
    }

    foreach(vtkVolume *prop, removedProps)
      m_picker->GetPickList()->AddItem(prop);
  }

  return selection;
}

//-----------------------------------------------------------------------------
void VolumetricRenderer::getPickCoordinates(double *point)
{
  m_picker->GetPickPosition(point);
}
