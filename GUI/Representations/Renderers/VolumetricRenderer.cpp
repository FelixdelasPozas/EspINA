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

namespace EspINA
{
  //-----------------------------------------------------------------------------
  template<class T>
  VolumetricRenderer<T>::VolumetricRenderer(QObject* parent)
  : RepresentationRenderer(parent)
  , m_picker(vtkSmartPointer<vtkVolumePicker>::New())
  {
    m_picker->PickFromListOn();
    m_picker->SetPickClippingPlanes(false);
    m_picker->SetPickCroppingPlanes(false);
    m_picker->SetPickTextureData(false);
    m_picker->SetTolerance(0);
  }

  //-----------------------------------------------------------------------------
  template<class T>
  VolumetricRenderer<T>::~VolumetricRenderer()
  {
    for(auto item: m_representations.keys())
    {
      if (m_enable)
        for(auto rep: m_representations[item])
        {
          for(auto prop: rep->getActors())
          {
            m_view->removeActor(prop);
            m_picker->DeletePickList(prop);
          }
        }

      m_representations[item].clear();
    }

    m_representations.clear();
  }

  //-----------------------------------------------------------------------------
  template<class T>
  void VolumetricRenderer<T>::addRepresentation(ViewItemAdapterPtr item, RepresentationSPtr rep)
  {
    VolumetricRepresentationSPtr<T> volume = std::dynamic_pointer_cast<VolumetricRepresentation<T>>(rep);
    if (volume.get() != nullptr)
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
  template<class T>
  void VolumetricRenderer<T>::removeRepresentation(RepresentationSPtr rep)
  {
    VolumetricRepresentationSPtr<T> volume = std::dynamic_pointer_cast<VolumetricRepresentation<T>>(rep);
    if (volume.get() != nullptr)
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
  template<class T>
  bool VolumetricRenderer<T>::hasRepresentation(RepresentationSPtr rep) const
  {
    for(auto item: m_representations.keys())
      if (m_representations[item].contains(rep))
        return true;

    return false;
  }

  //-----------------------------------------------------------------------------
  template<class T>
  bool VolumetricRenderer<T>::managesRepresentation(const QString &repName) const
  {
    return (repName == VolumetricRepresentation<T>::TYPE);
  }

  //-----------------------------------------------------------------------------
  template<class T>
  void VolumetricRenderer<T>::hide()
  {
      if (!m_enable)
        return;

      for(auto item: m_representations.keys())
        for(auto rep: m_representations[item])
          for(auto prop: rep->getActors())
          {
            m_view->removeActor(prop);
            m_picker->DeletePickList(prop);
          }

      emit renderRequested();
  }

  //-----------------------------------------------------------------------------
  template<class T>
  void VolumetricRenderer<T>::show()
  {
    if (m_enable)
      return;

    QApplication::setOverrideCursor(Qt::WaitCursor);
    for(auto item: m_representations.keys())
      for(auto rep: m_representations[item])
        for(auto prop: rep->getActors())
        {
          m_view->addActor(prop);
          m_picker->AddPickList(prop);
        }

    QApplication::restoreOverrideCursor();
    emit renderRequested();
  }

  //-----------------------------------------------------------------------------
  template<class T>
  ViewItemAdapterList VolumetricRenderer<T>::pick(int x, int y, Nm z, vtkSmartPointer<vtkRenderer> renderer, RenderableItems itemType, bool repeat)
  {
    ViewItemAdapterList selection;
    QList<vtkVolume *> removedProps;

    if (!renderer || !renderer.GetPointer() || !itemType.testFlag(EspINA::SEGMENTATION))
      return selection;

    while (m_picker->Pick(x, y, 0, renderer))
    {
      vtkVolume *pickedProp = m_picker->GetVolume();
      Q_ASSERT(pickedProp);

      m_picker->DeletePickList(pickedProp);
      removedProps << pickedProp;

      for(auto item: m_representations.keys())
        for(auto rep: m_representations[item])
          if (rep->isVisible() && rep->hasActor(pickedProp) && !selection.contains(item))
          {
            selection << item;

            if (!repeat)
            {
              for(auto actor: removedProps)
                m_picker->AddPickList(actor);

              return selection;
            }

            break;
          }
    }

    foreach(vtkVolume *prop, removedProps)
      m_picker->AddPickList(prop);

    return selection;
  }

  //-----------------------------------------------------------------------------
  template<class T>
  NmVector3 VolumetricRenderer<T>::pickCoordinates() const
  {
    Nm point[3];
    m_picker->GetPickPosition(point);

    return NmVector3{point[0], point[1], point[2]};
  }

} // namespace EspINA

