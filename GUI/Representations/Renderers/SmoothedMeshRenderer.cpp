/*
 <one line to give the program's name and a brief idea of what it does.>
 Copyright (C) 2013 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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

// VTK
#include <vtkPropPicker.h>

namespace EspINA
{
  //-----------------------------------------------------------------------------
  SmoothedMeshRenderer::SmoothedMeshRenderer(QObject* parent)
  : MeshRenderer(parent)
  {
  }

  //-----------------------------------------------------------------------------
  void SmoothedMeshRenderer::addRepresentation(ViewItemAdapterPtr item, RepresentationSPtr rep)
  {
    auto mesh = std::dynamic_pointer_cast<SmoothedMeshRepresentation>(rep);
    if (mesh.get() != nullptr)
    {
      if (m_representations.keys().contains(item))
        m_representations[item] << rep;
      else
      {
        RepresentationSList list;
        list << rep;
        m_representations.insert(item, list);
      }

      if (m_enable)
        for(auto prop: rep->getActors())
        {
          m_view->addActor(prop);
          m_picker->AddPickList(prop);
        }
    }
  }

  //-----------------------------------------------------------------------------
  void SmoothedMeshRenderer::removeRepresentation(RepresentationSPtr rep)
  {
    auto mesh = std::dynamic_pointer_cast<SmoothedMeshRepresentation>(rep);
    if (mesh.get() != nullptr)
    {
      for(auto item: m_representations.keys())
        if (m_representations[item].contains(rep))
        {
          if (m_enable)
            for(auto prop: rep->getActors())
            {
              m_view->removeActor(prop);
              m_picker->DeletePickList(prop);
            }

          m_representations[item].removeAll(rep);

          if (m_representations[item].isEmpty())
            m_representations.remove(item);
        }
    }
  }

  //-----------------------------------------------------------------------------
  bool SmoothedMeshRenderer::managesRepresentation(const QString &repName) const
  {
    return (repName == SmoothedMeshRepresentation::TYPE);
  }
  
} // namespace EspINA
