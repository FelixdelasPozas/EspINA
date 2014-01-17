/*
 <one line to give the program's name and a brief idea of what it does.>
 Copyright (C) 2013 F�lix de las Pozas �lvarez <felixdelaspozas@gmail.com>

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

#ifndef ESPINA_VOLUMETRIC_GPU_RENDERER_H
#define ESPINA_VOLUMETRIC_GPU_RENDERER_H

#include "EspinaGUI_Export.h"

// EspINA
#include "VolumetricRenderer.h"
#include "GUI/View/RenderView.h"
#include <GUI/Representations/VolumetricGPURepresentation.h>

// VTK
#include <vtkVolume.h>

// Qt
#include <QApplication>

namespace EspINA
{
  template<class T>
  class EspinaGUI_EXPORT VolumetricGPURenderer
  : public VolumetricRenderer<T>
  {
    public:
      explicit VolumetricGPURenderer(QObject* parent = 0);
      virtual ~VolumetricGPURenderer();

      virtual const QIcon icon()      const {return QIcon(":/espina/voxelGPU.png");}
      virtual const QString name()    const {return "Volumetric GPU";}
      virtual const QString tooltip() const {return "Segmentation's GPU Rendered Volumes";}

      virtual void addRepresentation(ViewItemAdapterPtr item, RepresentationSPtr rep);
      virtual void removeRepresentation(RepresentationSPtr rep);
      virtual bool managesRepresentation(RepresentationSPtr rep);

      virtual RendererSPtr clone() {return RendererSPtr(new VolumetricGPURenderer());}
  };


} // namespace EspINA

#include "VolumetricGPURenderer.cpp"

#endif // ESPINA_VOLUMETRIC_GPU_RENDERER_H
