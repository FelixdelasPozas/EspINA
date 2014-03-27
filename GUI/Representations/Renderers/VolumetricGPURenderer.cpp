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

namespace EspINA
{
  //-----------------------------------------------------------------------------
  template<class T>
  VolumetricGPURenderer<T>::VolumetricGPURenderer(QObject* parent)
      : VolumetricRenderer<T>(parent)
  {
  }

  //-----------------------------------------------------------------------------
  template<class T>
  VolumetricGPURenderer<T>::~VolumetricGPURenderer()
  {
  }

  //-----------------------------------------------------------------------------
  template<class T>
  void VolumetricGPURenderer<T>::addRepresentation(ViewItemAdapterPtr item, RepresentationSPtr rep)
  {
    VolumetricGPURepresentationSPtr<T> volume = std::dynamic_pointer_cast < VolumetricGPURepresentation < T >> (rep);
    if (volume.get() != nullptr)
    {
      if (this->m_representations.keys().contains(item))
        this->m_representations[item] << rep;
      else
      {
        RepresentationSList list;
        list << rep;
        this->m_representations.insert(item, list);
      }

      if (this->m_enable)
        for (auto prop : rep->getActors())
        {
          this->m_view->addActor(prop);
          this->m_picker->AddPickList(prop);
        }
    }
  }

  //-----------------------------------------------------------------------------
  template<class T>
  void VolumetricGPURenderer<T>::removeRepresentation(RepresentationSPtr rep)
  {
    VolumetricGPURepresentationSPtr<T> volume = std::dynamic_pointer_cast < VolumetricGPURepresentation < T >> (rep);
    if (volume.get() != nullptr)
    {
      for (auto item : this->m_representations.keys())
        if (this->m_representations[item].contains(rep))
        {
          if (this->m_enable)
            for (auto prop : rep->getActors())
            {
              this->m_view->removeActor(prop);
              this->m_picker->DeletePickList(prop);
            }
          
          this->m_representations[item].removeAll(rep);

          if (this->m_representations[item].isEmpty())
            this->m_representations.remove(item);
        }
    }
  }

  //-----------------------------------------------------------------------------
  template<class T>
  bool VolumetricGPURenderer<T>::managesRepresentation(const QString &repName) const
  {
    return (repName == VolumetricGPURepresentation<T>::TYPE);
  }

}
