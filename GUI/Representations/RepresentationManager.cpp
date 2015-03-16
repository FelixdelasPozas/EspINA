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

// ESPINA
#include "RepresentationManager.h"
#include <GUI/View/RenderView.h>

#include <vtkProp.h>

using namespace ESPINA;

//-----------------------------------------------------------------------------
RepresentationManager::RepresentationManager(ViewTypeFlags supportedViews)
: m_showRepresentations{false}
, m_requiresRender{false}
, m_view{nullptr}
, m_supportedViews{supportedViews}
, m_lastRequestTime{1}
, m_lastRenderRequestTime{0}
{
}

//-----------------------------------------------------------------------------
void RepresentationManager::setName(const QString &name)
{
  m_name = name;
}

//-----------------------------------------------------------------------------
QString RepresentationManager::name() const
{
  return m_name;
}

//-----------------------------------------------------------------------------
void RepresentationManager::setDescription(const QString &description)
{
  m_description = description;
}

//-----------------------------------------------------------------------------
QString RepresentationManager::description() const
{
  return m_description;
}

//-----------------------------------------------------------------------------
void RepresentationManager::setIcon(const QIcon &icon)
{
  m_icon = icon;
}

//-----------------------------------------------------------------------------
QIcon RepresentationManager::icon() const
{
  return m_icon;
}

//-----------------------------------------------------------------------------
void RepresentationManager::setView(RenderView *view)
{
  m_view = view;

  if (m_showRepresentations)
  {
    onShow();

    setCrosshair(m_crosshair, m_lastRequestTime);

    emit renderRequested();
  }
}

//-----------------------------------------------------------------------------
void RepresentationManager::show()
{
  for (auto child : m_childs)
  {
    child->show();
  }

  m_showRepresentations  = true;

  if (m_view)
  {
    onShow();

    setCrosshair(m_crosshair, m_lastRequestTime);

    emit renderRequested();
  }
}

//-----------------------------------------------------------------------------
void RepresentationManager::hide()
{
  for (auto child : m_childs)
  {
    child->hide();
  }

  m_showRepresentations = false;

  if (m_view)
  {
    emit renderRequested();

    onHide();
  }
}

//-----------------------------------------------------------------------------
bool RepresentationManager::isActive()
{
  return m_showRepresentations;
}

//-----------------------------------------------------------------------------
bool RepresentationManager::requiresRender() const
{
  return m_requiresRender;
}

//-----------------------------------------------------------------------------
void RepresentationManager::addPool(RepresentationPoolSPtr pool)
{
  m_pools << pool;
}

//-----------------------------------------------------------------------------
RepresentationManagerSPtr RepresentationManager::clone()
{
  auto child = cloneImplementation();

  m_childs << child;

  return child;
}

//-----------------------------------------------------------------------------
void RepresentationManager::onCrosshairChanged(NmVector3 crosshair, TimeStamp time)
{
  m_crosshair       = crosshair;
  m_lastRequestTime = time;

  if (m_showRepresentations)
  {
    setCrosshair(m_crosshair, time);
  }
}

//-----------------------------------------------------------------------------
void RepresentationManager::setRepresentationsVisibility(bool value)
{
  if (m_showRepresentations != value)
  {
    if (value)
    {
      show();
    }
    else
    {
      hide();
    }
  }
}

//-----------------------------------------------------------------------------
RepresentationPoolSList RepresentationManager::managedPools() const
{
  return m_pools;
}

//-----------------------------------------------------------------------------
void RepresentationManager::emitRenderRequest(TimeStamp time)
{
  if(time > m_lastRenderRequestTime)
  {
    m_requiresRender        = true;
    m_lastRenderRequestTime = time;

    emit renderRequested();
  }
}

//-----------------------------------------------------------------------------
void RepresentationManager::invalidateRepresentations()
{
  m_lastRenderRequestTime = 0;
}
