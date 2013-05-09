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
  void MeshRenderer::addRepresentation(GraphicalRepresentationSPtr rep)
  {
    SimpleMeshRepresentationSPtr mesh = boost::dynamic_pointer_cast<SimpleMeshRepresentation>(rep);
    if ((mesh.get() == NULL) || m_representations.contains(rep))
      return;

    m_representations << rep;
  }

  //-----------------------------------------------------------------------------
  void MeshRenderer::removeRepresentation(GraphicalRepresentationSPtr rep)
  {
    SimpleMeshRepresentationSPtr mesh = boost::dynamic_pointer_cast<SimpleMeshRepresentation>(rep);
    if (!m_representations.contains(rep) || (mesh.get() == NULL))
      return;

    m_representations.removeAll(rep);
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
    return m_representations.contains(rep);
  }

  //-----------------------------------------------------------------------------
  void MeshRenderer::hide()
  {
    if (!m_enable)
      return;

    foreach(GraphicalRepresentationSPtr rep, m_representations)
    {
      rep->setVisible(false);
      rep->updateRepresentation();
    }

    emit renderRequested();
  }

  //-----------------------------------------------------------------------------
  void MeshRenderer::show()
  {
     if (m_enable)
       return;

     foreach(GraphicalRepresentationSPtr rep, m_representations)
     {
       rep->setVisible(true);
       rep->updateRepresentation();
     }

     emit renderRequested();
  }

  //-----------------------------------------------------------------------------
  unsigned int MeshRenderer::getNumberOfvtkActors()
  {
    unsigned int returnVal = 0;
    foreach(GraphicalRepresentationSPtr rep, m_representations)
      if (rep->isVisible()) ++returnVal;

    return returnVal;
  }

  //-----------------------------------------------------------------------------
  GraphicalRepresentationSList MeshRenderer::pick(int x, int y, bool repeat)
  {
    QList<vtkProp *> removedProps;
    GraphicalRepresentationSList selection;

    if (m_renderer.GetPointer() != NULL)
    {
      while (m_picker->Pick(x,y,0, m_renderer))
      {
        vtkProp *pickedProp = m_picker->GetViewProp();
        Q_ASSERT(pickedProp);

        m_picker->GetPickList()->RemoveItem(pickedProp);
        removedProps << pickedProp;

        foreach(GraphicalRepresentationSPtr rep, m_representations)
          if (rep->hasActor(pickedProp))
          {
            selection << rep;
            break;
          }

        if (!repeat)
          break;
      }
    }

    foreach(vtkProp *actor, removedProps)
      m_picker->GetPickList()->AddItem(actor);

    return selection;
  }

  //-----------------------------------------------------------------------------
  void MeshRenderer::getPickCoordinates(Nm *point)
  {
    m_picker->GetPickPosition(point);
  }


} // namespace EspINA

