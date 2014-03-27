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

#ifndef ESPINA_SMOOTHED_MESH_RENDERER_H_
#define ESPINA_SMOOTHED_MESH_RENDERER_H_

#include "EspinaGUI_Export.h"

// EspINA
#include "MeshRenderer.h"

namespace EspINA
{
  class ViewManager;

  class EspinaGUI_EXPORT SmoothedMeshRenderer
  : public MeshRenderer
  {
    public:
      explicit SmoothedMeshRenderer(QObject* parent = 0);

      virtual const QIcon icon()      const { return QIcon(":/espina/smoothedmesh.png"); }
      virtual const QString name()    const { return "Smoothed Mesh"; }
      virtual const QString tooltip() const { return "Segmentation's Smoothed Meshes"; }

      virtual void addRepresentation(ViewItemAdapterPtr item, RepresentationSPtr rep);
      virtual void removeRepresentation(RepresentationSPtr rep);
      virtual bool managesRepresentation(const QString &repName) const;

      virtual RendererSPtr clone()         { return RendererSPtr(new SmoothedMeshRenderer()); }
  };

} /* namespace EspINA */

#endif /* ESPINA_SMOOTHED_MESH_RENDERER_H_ */
