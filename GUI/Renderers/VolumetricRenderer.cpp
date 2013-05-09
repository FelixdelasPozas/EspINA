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
  void VolumetricRenderer::addRepresentation(GraphicalRepresentationSPtr rep)
  {
    VolumeRaycastRepresentationSPtr volume = boost::dynamic_pointer_cast<VolumeRaycastRepresentation>(rep);
    if ((volume.get() == NULL) || m_representations.contains(rep))
      return;

    m_representations << rep;
  }

  //-----------------------------------------------------------------------------
  void VolumetricRenderer::removeRepresentation(GraphicalRepresentationSPtr rep)
  {
    VolumeRaycastRepresentationSPtr volume = boost::dynamic_pointer_cast<VolumeRaycastRepresentation>(rep);
    if (!m_representations.contains(rep) || (volume.get() == NULL))
      return;

    m_representations.removeAll(rep);
  }

  //-----------------------------------------------------------------------------
  bool VolumetricRenderer::hasRepresentation(GraphicalRepresentationSPtr rep)
  {
    return m_representations.contains(rep);
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

      foreach(GraphicalRepresentationSPtr rep, m_representations)
      {
        rep->setVisible(false);
        rep->updateRepresentation();
      }

      emit renderRequested();
  }

  //-----------------------------------------------------------------------------
  void VolumetricRenderer::show()
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
  GraphicalRepresentationSList VolumetricRenderer::pick(int x, int y, bool repeat)
  {
    GraphicalRepresentationSList selection;
    QList<vtkVolume *> removedProps;

    if (m_renderer)
    {
      while (m_picker->Pick(x, y, 0, m_renderer))
      {
        vtkVolume *pickedProp = m_picker->GetVolume();
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

