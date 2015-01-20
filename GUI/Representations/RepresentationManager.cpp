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

using namespace ESPINA;

//-----------------------------------------------------------------------------
RepresentationManager::RepresentationManager(ViewTypeFlags flags)
: m_flags{flags}
, m_view{nullptr}
, m_displayRepresentations{false}
{
}

//-----------------------------------------------------------------------------
ViewTypeFlags RepresentationManager::supportedViews() const
{
  return m_flags;
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
void RepresentationManager::show()
{
  for (auto child : m_childs)
  {
    child->show();
  }

  m_displayRepresentations = true;

  updateRepresentationActors();
}

//-----------------------------------------------------------------------------
void RepresentationManager::hide()
{
  for (auto child : m_childs)
  {
    child->hide();
  }

  m_displayRepresentations = false;

  updateRepresentationActors();
}


//-----------------------------------------------------------------------------
void RepresentationManager::updateRepresentationActors()
{
  if (m_view != nullptr)
  {
    updateRepresentationImplementation(m_view, m_displayRepresentations);

    emit renderRequested();
  }
}

//-----------------------------------------------------------------------------
RepresentationManagerSPtr RepresentationManager::clone()
{
  auto child = cloneImpelementation();

  m_childs << child;

  return child;
}

//-----------------------------------------------------------------------------
void RepresentationManager::setRepresentationsVisibility(bool value)
{
  if (m_displayRepresentations != value)
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