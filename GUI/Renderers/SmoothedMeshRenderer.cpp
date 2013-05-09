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
#include "GUI/Representations/SmoothedMeshRepresentation.h"

namespace EspINA
{
  //-----------------------------------------------------------------------------
  SmoothedMeshRenderer::SmoothedMeshRenderer(QObject* parent)
  : MeshRenderer(parent)
  {
  }

  //-----------------------------------------------------------------------------
  void SmoothedMeshRenderer::addRepresentation(GraphicalRepresentationSPtr rep)
  {
    SmoothedMeshRepresentationSPtr mesh = boost::dynamic_pointer_cast<SmoothedMeshRepresentation>(rep);
    if ((mesh.get() == NULL) || m_representations.contains(rep))
      return;

    m_representations << rep;
  }

  //-----------------------------------------------------------------------------
  void SmoothedMeshRenderer::removeRepresentation(GraphicalRepresentationSPtr rep)
  {
    SmoothedMeshRepresentationSPtr mesh = boost::dynamic_pointer_cast<SmoothedMeshRepresentation>(rep);
    if (!m_representations.contains(rep) || (mesh.get() == NULL))
      return;

    m_representations.removeAll(rep);
  }

  //-----------------------------------------------------------------------------
  bool SmoothedMeshRenderer::managesRepresentation(GraphicalRepresentationSPtr rep)
  {
    SmoothedMeshRepresentationSPtr mesh = boost::dynamic_pointer_cast<SmoothedMeshRepresentation>(rep);
    return (mesh.get() != NULL);
  }
  
} // namespace EspINA

