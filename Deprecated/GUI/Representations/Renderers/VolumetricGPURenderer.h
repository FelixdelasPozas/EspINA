/*

 Copyright (C) 2014 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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

#ifndef ESPINA_VOLUMETRIC_GPU_RENDERER_H
#define ESPINA_VOLUMETRIC_GPU_RENDERER_H

// ESPINA
#include <Deprecated/GUI/Representations/Renderers/VolumetricRenderer.h>
#include <Deprecated/GUI/Representations/VolumetricGPURepresentation.hxx>
#include "GUI/View/RenderView.h"
#include <vtkVolume.h>

// Qt
#include <QApplication>

namespace ESPINA
{
  template<class T>
  class VolumetricGPURenderer
  : public VolumetricRenderer<T>
  {
    public:
  		/** \brief VolumetricGPURenderer class constructor.
  		 * \param[in] parent, raw pointer of the QObject parent of this one.
  		 *
  		 */
      explicit VolumetricGPURenderer(QObject* parent = nullptr);

  		/** \brief VolumetricGPURenderer class virtual destructor.
  		 *
  		 */
      virtual ~VolumetricGPURenderer();

      virtual const QIcon icon() const
      { return QIcon(":/espina/voxelGPU.png"); }

      virtual const QString name() const
      { return "Volumetric GPU"; }

      virtual const QString tooltip() const
      { return "Segmentation's GPU Rendered Volumes"; }

      virtual void addRepresentation(ViewItemAdapterPtr item, RepresentationSPtr rep);

      virtual void removeRepresentation(RepresentationSPtr rep);

      virtual bool managesRepresentation(const QString &representationType) const;

      virtual RendererSPtr clone() const {return RendererSPtr(new VolumetricGPURenderer());}
  };

  //-----------------------------------------------------------------------------
  template<class T>
  VolumetricGPURenderer<T>::VolumetricGPURenderer(QObject* parent)
  : VolumetricRenderer<T>{parent}
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
  bool VolumetricGPURenderer<T>::managesRepresentation(const QString &repType) const
  {
    return (repType == VolumetricGPURepresentation<T>::TYPE);
  }

} // namespace ESPINA

#endif // ESPINA_VOLUMETRIC_GPU_RENDERER_H
