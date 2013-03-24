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

// EspINA
#include "MeshRenderer.h"
#include <Core/Model/Segmentation.h>
#include <Core/ColorEngines/IColorEngine.h>
#include "GUI/ViewManager.h"

// VTK
#include <vtkRenderWindow.h>
#include <vtkSmartPointer.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkProperty.h>
#include <vtkImageConstantPad.h>
#include <vtkAlgorithm.h>
#include <vtkMath.h>

// Qt
#include <QApplication>

using namespace EspINA;

//-----------------------------------------------------------------------------
MeshRenderer::MeshRenderer(ViewManager* vm, QObject* parent)
: IRenderer(parent)
, m_viewManager(vm)
{
}

//-----------------------------------------------------------------------------
bool MeshRenderer::addItem(ModelItemPtr item)
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
        m_renderer->RemoveActor(this->m_segmentations[seg].actor);
      m_segmentations[seg].actor->Delete();
      m_segmentations[seg].actorPropertyBackup = NULL;
      m_segmentations.remove(item);

      m_mappers[seg] = NULL;
      m_mappers.remove(item);
    }

  QColor color = m_viewManager->color(seg);

  m_mappers[seg] = vtkSmartPointer<vtkPolyDataMapper>::New();
  m_mappers[seg]->ReleaseDataFlagOn();
  m_mappers[seg]->ImmediateModeRenderingOn();
  m_mappers[seg]->ScalarVisibilityOff();
  m_mappers[seg]->SetInputConnection(seg->volume()->toMesh());

  vtkActor *actor = vtkActor::New();
  actor->SetMapper(m_mappers[seg]);
  double rgb[3] = { color.redF(), color.greenF(), color.blueF() };
  double hsv[3] = { 0.0, 0.0, 0.0 };
  vtkMath::RGBToHSV(rgb,hsv);
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
  memcpy(m_segmentations[seg].extent, extent, 6*sizeof(int));

  if (m_enable)
  {
    m_segmentations[seg].visible = true;
    m_renderer->AddActor(actor);
  }

  if (m_segmentations[seg].overridden)
    createHierarchyProperties(seg);

  m_segmentations[seg].actor->Modified();

  connect(item, SIGNAL(modified(ModelItemPtr)), this, SLOT(updateItem(ModelItemPtr)));

  return true;
}

//-----------------------------------------------------------------------------
bool MeshRenderer::updateItem(ModelItemPtr item, bool forced)
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
      rep.actor->Modified();
      m_renderer->AddActor(rep.actor);
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
      rep.visible = false;
      return true;
    }
    return false;
  }

  // check if the beginning of the pipeline has changed
  if (m_mappers[seg]->GetInputConnection(0,0) != seg->volume()->toMesh())
  {
    m_mappers[seg]->SetInputConnection(seg->volume()->toMesh());
    m_mappers[seg]->Update();

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
bool MeshRenderer::removeItem(ModelItemPtr item)
{
   if (EspINA::SEGMENTATION != item->type())
     return false;

   SegmentationPtr seg = segmentationPtr(item);
   if(!m_segmentations.contains(seg))
     return false;

   if (m_segmentations[seg].visible)
     m_renderer->RemoveActor(m_segmentations[seg].actor);

   if (m_segmentations[seg].actorPropertyBackup)
     m_segmentations[seg].actorPropertyBackup = NULL;

   m_segmentations[seg].actor->Delete();
   m_segmentations.remove(seg);

   m_mappers[seg] = NULL;
   m_mappers.remove(seg);

   disconnect(item, SIGNAL(modified(ModelItemPtr)), this, SLOT(updateItem(ModelItemPtr)));

   return true;
}

//-----------------------------------------------------------------------------
void MeshRenderer::hide()
{
  if (!m_enable)
    return;

  QMap<ModelItemPtr, Representation>::iterator it;

  for (it = m_segmentations.begin(); it != m_segmentations.end(); ++it)
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
  if (m_enable)
    return;

  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

  QMap<ModelItemPtr, Representation>::iterator it;

  for (it = m_segmentations.begin(); it != m_segmentations.end(); ++it)
  {
    SegmentationPtr seg = segmentationPtr(it.key());
    if (seg->visible())
      updateItem(seg, true);
  }

  emit renderRequested();
  QApplication::restoreOverrideCursor();
}

//-----------------------------------------------------------------------------
unsigned int MeshRenderer::getNumberOfvtkActors()
{
  unsigned int numActors = 0;

  QMap<ModelItemPtr, Representation>::iterator it;
  for (it = m_segmentations.begin(); it != m_segmentations.end(); ++it)
    if ((*it).visible)
      numActors++;

  return numActors;
}

//-----------------------------------------------------------------------------
void MeshRenderer::createHierarchyProperties(SegmentationPtr seg)
{
  m_segmentations[seg].actorPropertyBackup = m_segmentations[seg].actor->GetProperty();
  m_segmentations[seg].overridden = true;
  m_segmentations[seg].renderingType = seg->getHierarchyRenderingType();
  vtkSmartPointer<vtkProperty> actorProperty = vtkSmartPointer<vtkProperty>::New();

  QColor color = m_segmentations[seg].color;
  double rgb[3] = { color.redF(), color.greenF(), color.blueF() };
  double hsv[3] = { 0.0, 0.0, 0.0 };
  vtkMath::RGBToHSV(rgb,hsv);
  hsv[2] = (seg->isSelected() ? 1.0 : 0.6);
  vtkMath::HSVToRGB(hsv, rgb);
  actorProperty->SetColor(rgb[0], rgb[1], rgb[2]);
  actorProperty->SetSpecular(0.2);

  switch(m_segmentations[seg].renderingType)
  {
    case HierarchyItem::Opaque:
      actorProperty->SetOpacity(1.0);
      if (m_enable && !m_segmentations[seg].visible)
      {
        m_segmentations[seg].visible = true;
        m_renderer->AddActor(m_segmentations[seg].actor);
      }
      break;
    case HierarchyItem::Translucent:
      actorProperty->SetOpacity(0.3);
      if (m_enable && !m_segmentations[seg].visible)
      {
        m_segmentations[seg].visible = true;
        m_renderer->AddActor(m_segmentations[seg].actor);
      }
      break;
    case HierarchyItem::Hidden:
      if (m_enable && m_segmentations[seg].visible)
      {
        m_segmentations[seg].visible = false;
        m_renderer->RemoveActor(m_segmentations[seg].actor);
      }
      break;
    case HierarchyItem::Undefined:
      break;
    default:
      Q_ASSERT(false);
      break;
  }

  m_segmentations[seg].actor->SetProperty(actorProperty);
}

//-----------------------------------------------------------------------------
bool MeshRenderer::updateHierarchyProperties(SegmentationPtr seg)
{
  Q_ASSERT(m_segmentations[seg].actorPropertyBackup != NULL);
  bool updated = false;

  vtkSmartPointer<vtkProperty> actorProperty = NULL;
  if (seg->OverridesRendering() != m_segmentations[seg].overridden)
  {
    actorProperty = m_segmentations[seg].actor->GetProperty();
    m_segmentations[seg].actor->SetProperty(m_segmentations[seg].actorPropertyBackup);
    m_segmentations[seg].actorPropertyBackup = actorProperty;
    m_segmentations[seg].actor->Modified();
    m_segmentations[seg].overridden = seg->OverridesRendering();
    updated = true;
  }

  if (!seg->OverridesRendering())
    return true;

  actorProperty = m_segmentations[seg].actor->GetProperty();
  if (m_segmentations[seg].color != m_viewManager->color(seg))
  {
    QColor color = m_segmentations[seg].color;
    double rgb[3] = { color.redF(), color.greenF(), color.blueF() };
    double hsv[3] = { 0.0, 0.0, 0.0 };
    vtkMath::RGBToHSV(rgb,hsv);
    hsv[2] = (seg->isSelected() ? 1.0 : 0.6);
    vtkMath::HSVToRGB(hsv, rgb);
    actorProperty->SetColor(rgb[0], rgb[1], rgb[2]);
    updated = true;
  }

  if (seg->getHierarchyRenderingType() != m_segmentations[seg].renderingType)
  {
    m_segmentations[seg].renderingType = seg->getHierarchyRenderingType();

    switch (m_segmentations[seg].renderingType)
    {
      case HierarchyItem::Opaque:
        actorProperty->SetOpacity(1.0);
        if (m_enable && !m_segmentations[seg].visible)
        {
          m_segmentations[seg].visible = true;
          m_renderer->AddActor(m_segmentations[seg].actor);
        }
        break;
      case HierarchyItem::Translucent:
        actorProperty->SetOpacity(0.3);
        if (m_enable && !m_segmentations[seg].visible)
        {
          m_segmentations[seg].visible = true;
          m_renderer->AddActor(m_segmentations[seg].actor);
        }
        break;
      case HierarchyItem::Hidden:
        if (m_enable && m_segmentations[seg].visible)
        {
          m_segmentations[seg].visible = false;
          m_renderer->RemoveActor(m_segmentations[seg].actor);
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

  return updated;
}
