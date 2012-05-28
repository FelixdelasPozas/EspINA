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

#include <model/Segmentation.h>
#include <pqOutputPort.h>
#include <pqPipelineSource.h>
#include <vtkSMProxyManager.h>
#include <vtkSMRepresentationProxy.h>
#include <ColorEngine.h>
#include <EspinaCore.h>
#include <pqSMAdaptor.h>
#include <vtkSMPropertyHelper.h>

//-----------------------------------------------------------------------------
bool VolumetricRenderer::addItem(ModelItem* item)
{
  if (ModelItem::SEGMENTATION != item->type())
    return false;

  Segmentation *seg = dynamic_cast<Segmentation *>(item);
  pqOutputPort      *oport = seg->outputPort();
  pqPipelineSource *source = oport->getSource();
  vtkSMProxyManager   *pxm = vtkSMProxyManager::GetProxyManager();

  vtkSMRepresentationProxy* repProxy = vtkSMRepresentationProxy::SafeDownCast(
    pxm->NewProxy("representations", "VolumetricRepresentation"));
  Q_ASSERT(repProxy);

  ColorEngine *engine = EspinaCore::instance()->colorSettings().engine();
  m_segmentations[seg].outport  = oport;
  m_segmentations[seg].proxy    = repProxy;
  m_segmentations[seg].selected = !seg->selected();
  m_segmentations[seg].visible  = seg->visible();
  m_segmentations[seg].color    = engine->color(seg);

  // Set the reprProxy's input.
  pqSMAdaptor::setInputProperty(repProxy->GetProperty("Input"),
				source->getProxy(),
				oport->getPortNumber());

  updateItem(seg);

  // Add the reprProxy to render module.
  pqSMAdaptor::addProxyProperty(
    m_view->GetProperty("Representations"), repProxy);
  m_view->UpdateVTKObjects();

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
  if (seg->outputPort() != rep.outport)
  {
    removeItem(seg);
    addItem(seg);
    updated = true;
  } else if (seg->selected() != rep.selected
          || seg->visible() != rep.visible
          || seg->data(Qt::DecorationRole).value<QColor>() != rep.color)
  {
    rep.selected = seg->selected();
    rep.visible  = seg->visible();
    rep.color = seg->data(Qt::DecorationRole).value<QColor>();
    //   repProxy->PrintSelf(std::cout,vtkIndent(0));
    double rgb[3] = {rep.color.redF(), rep.color.greenF(), rep.color.blueF()};
    vtkSMPropertyHelper(rep.proxy, "Color").Set(rgb, 3);
    vtkSMPropertyHelper(rep.proxy, "Opacity").Set(rep.selected?1.0:0.7);
    vtkSMPropertyHelper(rep.proxy, "Visibility").Set(rep.visible && m_enable);
    rep.proxy->UpdateVTKObjects();
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
  vtkSMRepresentationProxy *repProxy = m_segmentations[seg].proxy;
  // Remove the reprProxy to render module.
  pqSMAdaptor::removeProxyProperty(
    m_view->GetProperty("Representations"), repProxy);
  m_view->UpdateVTKObjects();
  repProxy->Delete();
  m_segmentations.remove(seg);

  return true;
}

//-----------------------------------------------------------------------------
void VolumetricRenderer::hide()
{
  foreach(Representation rep, m_segmentations)
  {
    vtkSMPropertyHelper(rep.proxy, "Visibility").Set(false);
    rep.proxy->UpdateVTKObjects();
  }
  emit renderRequested();
}

//-----------------------------------------------------------------------------
void VolumetricRenderer::show()
{
  foreach(Representation rep, m_segmentations)
  {
    vtkSMPropertyHelper(rep.proxy, "Visibility").Set(rep.visible);
    rep.proxy->UpdateVTKObjects();
  }
  emit renderRequested();
}