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
#include "VolumetricRenderer.h"
#include "GUI/QtWidget/EspinaRenderView.h"

// VTK
#include <vtkVolume.h>

namespace EspINA
{
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
  VolumetricRenderer::~VolumetricRenderer()
  {
    foreach(PickableItemPtr item, m_representations.keys())
    {
      foreach(GraphicalRepresentationSPtr rep, m_representations[item])
      {
        foreach(vtkProp *prop, rep->getActors())
        {
          m_renderer->RemoveActor(prop);
          m_picker->DeletePickList(prop);
        }
      }
      m_representations[item].clear();
    }

    m_representations.clear();
  }

  //-----------------------------------------------------------------------------
  void VolumetricRenderer::addRepresentation(PickableItemPtr item, GraphicalRepresentationSPtr rep)
  {
    VolumeRaycastRepresentationSPtr volume = boost::dynamic_pointer_cast<VolumeRaycastRepresentation>(rep);
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
        foreach(vtkProp3D *prop, rep->getActors())
        {
          m_view->addActor(prop);
          m_picker->AddPickList(prop);
        }
    }
  }

  //-----------------------------------------------------------------------------
  void VolumetricRenderer::removeRepresentation(GraphicalRepresentationSPtr rep)
  {
    VolumeRaycastRepresentationSPtr volume = boost::dynamic_pointer_cast<VolumeRaycastRepresentation>(rep);
    if (volume.get() != NULL)
    {
      foreach(PickableItemPtr item, m_representations.keys())
        if (m_representations[item].contains(rep))
        {
          if (m_enable)
            foreach(vtkProp3D *prop, rep->getActors())
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
  bool VolumetricRenderer::hasRepresentation(GraphicalRepresentationSPtr rep)
  {
    foreach(PickableItemPtr item, m_representations.keys())
      if (m_representations[item].contains(rep))
        return true;

    return false;
  }

  //-----------------------------------------------------------------------------
  bool VolumetricRenderer::managesRepresentation(GraphicalRepresentationSPtr rep)
  {
    VolumeRaycastRepresentationSPtr volume = boost::dynamic_pointer_cast<VolumeRaycastRepresentation>(rep);
    return (volume.get() != NULL);
  }

  //-----------------------------------------------------------------------------
  void VolumetricRenderer::hide()
  {
      if (!m_enable)
        return;

      foreach(PickableItemPtr item, m_representations.keys())
        foreach(GraphicalRepresentationSPtr rep, m_representations[item])
          foreach(vtkProp3D* prop, rep->getActors())
          {
            m_view->removeActor(prop);
            m_picker->DeletePickList(prop);
          }

      emit renderRequested();
  }

  //-----------------------------------------------------------------------------
  void VolumetricRenderer::show()
  {
    if (m_enable)
      return;

    foreach(PickableItemPtr item, m_representations.keys())
      foreach(GraphicalRepresentationSPtr rep, m_representations[item])
        foreach(vtkProp3D* prop, rep->getActors())
        {
          m_view->addActor(prop);
          m_picker->AddPickList(prop);
        }

    emit renderRequested();
  }

  //-----------------------------------------------------------------------------
  ViewManager::Selection VolumetricRenderer::pick(int x, int y, vtkSmartPointer<vtkRenderer> renderer, bool repeat)
  {
    ViewManager::Selection selection;
    QList<vtkVolume *> removedProps;

    if (!renderer || !renderer.GetPointer())
      renderer = m_renderer;

    if (renderer)
    {
      while (m_picker->Pick(x, y, 0, renderer))
      {
        vtkVolume *pickedProp = m_picker->GetVolume();
        Q_ASSERT(pickedProp);

        m_picker->GetPickList()->RemoveItem(pickedProp);
        removedProps << pickedProp;

        foreach(PickableItemPtr item, m_representations.keys())
          foreach(GraphicalRepresentationSPtr rep, m_representations[item])
            if (rep->isVisible() && rep->hasActor(pickedProp) && !selection.contains(item))
            {
              selection << item;

              if (!repeat)
              {
                m_picker->GetPickList()->AddItem(pickedProp);
                return selection;
              }

              break;
            }
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

} // namespace EspINA

