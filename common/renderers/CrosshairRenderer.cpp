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


#include "CrosshairRenderer.h"

#include <QDebug>

#include "common/model/Channel.h"
#include "common/model/Representation.h"

#include <pqOutputPort.h>
#include <pqPipelineSource.h>
#include <pqSMAdaptor.h>
#include <vtkSMPropertyHelper.h>
#include <vtkSMProxyManager.h>
#include <vtkSMRepresentationProxy.h>

//-----------------------------------------------------------------------------
bool CrosshairRenderer::addItem(ModelItem* item)
{
  if (ModelItem::CHANNEL != item->type())
    return false;

  Channel *channel = dynamic_cast<Channel *>(item);
  pqData volume = channel->representation("Volumetric")->output();
  pqOutputPort      *oport = volume.outputPort();
  pqPipelineSource *source = oport->getSource();
  vtkSMProxyManager   *pxm = vtkSMProxyManager::GetProxyManager();

  vtkSMRepresentationProxy* repProxy = vtkSMRepresentationProxy::SafeDownCast(
    pxm->NewProxy("representations", "CrosshairRepresentation"));
  Q_ASSERT(repProxy);
  m_channels[channel].proxy = repProxy;
  m_channels[channel].visible = !channel->isVisible();

  // Set repProxy's input.
  pqSMAdaptor::setInputProperty(repProxy->GetProperty("Input"),
				source->getProxy(),
				oport->getPortNumber());

  updateItem(item);

  // Add the reprProxy to renderer module.
  pqSMAdaptor::addProxyProperty(
    m_view->GetProperty("Representations"), repProxy);
  m_view->UpdateVTKObjects();

  return true;
}


//-----------------------------------------------------------------------------
bool CrosshairRenderer::updateItem(ModelItem* item)
{
  if (ModelItem::CHANNEL != item->type())
    return false;

  bool updated = false;
  Channel *channel = dynamic_cast<Channel *>(item);
  Q_ASSERT(m_channels.contains(channel));
  Representation &rep = m_channels[channel];
  double pos[3];
  channel->position(pos);
  //TODO: update if position changes
  if (channel->isVisible() != rep.visible)
  {
    rep.visible  = channel->isVisible();

    vtkSMPropertyHelper(rep.proxy, "Position").Set(pos,3);
    double color = channel->color();
    vtkSMPropertyHelper(rep.proxy, "Color").Set(&color,1);
    vtkSMPropertyHelper(rep.proxy, "Visibility").Set(rep.visible && m_enable);
    double opacity = 1.0;//suggestedChannelOpacity();
    vtkSMPropertyHelper(rep.proxy, "Opacity").Set(&opacity,1);
    rep.proxy->UpdateVTKObjects();
    updated = true;
  }

  return updated;
}

//-----------------------------------------------------------------------------
bool CrosshairRenderer::removeItem(ModelItem* item)
{
  if (ModelItem::CHANNEL != item->type())
    return false;

  Channel *channel = dynamic_cast<Channel *>(item);
  Q_ASSERT(m_channels.contains(channel));
  vtkSMRepresentationProxy *repProxy = m_channels[channel].proxy;
  // Remove the reprProxy to render module.
  pqSMAdaptor::removeProxyProperty(
    m_view->GetProperty("Representations"), repProxy);
  m_view->UpdateVTKObjects();
  repProxy->Delete();
  m_channels.remove(channel);

  return true;
}


//-----------------------------------------------------------------------------
void CrosshairRenderer::hide()
{
  foreach(Representation rep, m_channels)
  {
    vtkSMPropertyHelper(rep.proxy, "Visibility").Set(false);
    rep.proxy->UpdateVTKObjects();
  }
  emit renderRequested();
}

//-----------------------------------------------------------------------------
void CrosshairRenderer::show()
{
  foreach(Representation rep, m_channels)
  {
    vtkSMPropertyHelper(rep.proxy, "Visibility").Set(rep.visible);
    rep.proxy->UpdateVTKObjects();
  }
  emit renderRequested();
}


