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
#include <GUI/Representations/SimpleMeshRepresentation.h>
#include "GUI/QtWidget/EspinaRenderView.h"

// VTK
#include <vtkPropPicker.h>

namespace EspINA
{
  //-----------------------------------------------------------------------------
  MeshRenderer::MeshRenderer(QObject *parent)
  : IRenderer(parent)
  , m_picker(vtkSmartPointer<vtkPropPicker>::New())
  {
    m_picker->PickFromListOn();
  }

  //-----------------------------------------------------------------------------
  MeshRenderer::~MeshRenderer()
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
  void MeshRenderer::addRepresentation(PickableItemPtr item, GraphicalRepresentationSPtr rep)
  {
    SimpleMeshRepresentationSPtr mesh = boost::dynamic_pointer_cast<SimpleMeshRepresentation>(rep);
    if (mesh.get() != NULL)
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
        foreach(vtkProp3D* prop, rep->getActors())
        {
          m_view->addActor(prop);
          m_picker->AddPickList(prop);
        }
    }
  }

  //-----------------------------------------------------------------------------
  void MeshRenderer::removeRepresentation(GraphicalRepresentationSPtr rep)
  {
    SimpleMeshRepresentationSPtr mesh = boost::dynamic_pointer_cast<SimpleMeshRepresentation>(rep);
    if (mesh.get() != NULL)
    {
      foreach(PickableItemPtr item, m_representations.keys())
        if (m_representations[item].contains(rep))
        {
          foreach(vtkProp3D* prop, rep->getActors())
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
  bool MeshRenderer::managesRepresentation(GraphicalRepresentationSPtr rep)
  {
    SimpleMeshRepresentationSPtr mesh = boost::dynamic_pointer_cast<SimpleMeshRepresentation>(rep);
    return (mesh.get() != NULL);
  }

  //-----------------------------------------------------------------------------
  bool MeshRenderer::hasRepresentation(GraphicalRepresentationSPtr rep)
  {
    foreach (PickableItemPtr item, m_representations.keys())
      if (m_representations[item].contains(rep))
        return true;

    return false;
  }

  //-----------------------------------------------------------------------------
  void MeshRenderer::hide()
  {
    if (!m_enable)
      return;

    foreach (PickableItemPtr item, m_representations.keys())
      foreach(GraphicalRepresentationSPtr rep, m_representations[item])
        foreach(vtkProp3D* prop, rep->getActors())
        {
          m_view->removeActor(prop);
          m_picker->DeletePickList(prop);
        }

    emit renderRequested();
  }

  //-----------------------------------------------------------------------------
  void MeshRenderer::show()
  {
     if (m_enable)
       return;

     foreach (PickableItemPtr item, m_representations.keys())
       foreach(GraphicalRepresentationSPtr rep, m_representations[item])
         foreach(vtkProp3D* prop, rep->getActors())
         {
           m_view->addActor(prop);
           m_picker->AddPickList(prop);
         }

     emit renderRequested();
  }

  //-----------------------------------------------------------------------------
  unsigned int MeshRenderer::getNumberOfvtkActors()
  {
    unsigned int returnVal = 0;
    foreach (PickableItemPtr item, m_representations.keys())
      foreach(GraphicalRepresentationSPtr rep, m_representations[item])
        if (rep->isVisible()) ++returnVal;

    return returnVal;
  }

  //-----------------------------------------------------------------------------
  ViewManager::Selection MeshRenderer::pick(int x, int y, vtkSmartPointer<vtkRenderer> renderer, RenderabledItems itemType,  bool repeat)
  {
    ViewManager::Selection selection;
    QList<vtkProp *> removedProps;

    if (!renderer || !renderer.GetPointer())
      renderer = m_renderer;

    if (renderer.GetPointer() != NULL && itemType.testFlag(EspINA::SEGMENTATION))
    {
      while (m_picker->Pick(x,y,0, renderer))
      {
        vtkProp *pickedProp = m_picker->GetViewProp();
        Q_ASSERT(pickedProp);

        m_picker->DeletePickList(pickedProp);
        removedProps << pickedProp;

        foreach(PickableItemPtr item, m_representations.keys())
          foreach(GraphicalRepresentationSPtr rep, m_representations[item])
            if (rep->isVisible() && rep->hasActor(pickedProp) && !selection.contains(item))
            {
              selection << item;

              if (!repeat)
              {
                m_picker->AddPickList(pickedProp);
                return selection;
              }

              break;
            }
      }

      foreach(vtkProp *actor, removedProps)
        m_picker->AddPickList(actor);
    }

    return selection;
  }

  //-----------------------------------------------------------------------------
  void MeshRenderer::getPickCoordinates(Nm *point)
  {
    m_picker->GetPickPosition(point);
  }

} // namespace EspINA

