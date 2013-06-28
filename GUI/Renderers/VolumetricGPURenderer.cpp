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
#include "GUI/Renderers/VolumetricGPURenderer.h"
#include "GUI/Representations/VolumeGPURepresentation.h"

namespace EspINA
{
  //-----------------------------------------------------------------------------
  VolumetricGPURenderer::VolumetricGPURenderer(QObject* parent)
  : VolumetricRenderer(parent)
  {
  }
  
  //-----------------------------------------------------------------------------
  VolumetricGPURenderer::~VolumetricGPURenderer()
  {
  }
  
  //-----------------------------------------------------------------------------
  void VolumetricGPURenderer::addRepresentation(PickableItemPtr item, GraphicalRepresentationSPtr rep)
  {
    VolumeGPURepresentationSPtr volume = boost::dynamic_pointer_cast<VolumeGPURaycastRepresentation>(rep);
    if (volume.get() != NULL)
    {
      if (m_representations.keys().contains(item))
        m_representations[item] << rep;
      else
      {
        GraphicalRepresentationSList list;
        list << rep;
        m_representations.insert(item, list);
      }

      if (m_enable)
        foreach(vtkProp *prop, rep->getActors())
        {
          m_view->addActor(prop);
          m_picker->AddPickList(prop);
        }
    }
  }

  //-----------------------------------------------------------------------------
  void VolumetricGPURenderer::removeRepresentation(GraphicalRepresentationSPtr rep)
  {
    VolumeGPURepresentationSPtr volume = boost::dynamic_pointer_cast<VolumeGPURaycastRepresentation>(rep);
    if (volume.get() != NULL)
    {
      foreach(PickableItemPtr item, m_representations.keys())
        if (m_representations[item].contains(rep))
        {
          if (m_enable)
            foreach(vtkProp *prop, rep->getActors())
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
  bool VolumetricGPURenderer::managesRepresentation(GraphicalRepresentationSPtr rep)
  {
    VolumeGPURepresentationSPtr volume = boost::dynamic_pointer_cast<VolumeGPURaycastRepresentation>(rep);
    return (volume.get() != NULL);
  }


} /* namespace EspINA */
