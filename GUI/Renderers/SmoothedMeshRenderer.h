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

#ifndef SMOOTHEDMESHRENDERER_H_
#define SMOOTHEDMESHRENDERER_H_

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

      virtual void addRepresentation(PickableItemPtr item, GraphicalRepresentationSPtr rep);
      virtual void removeRepresentation(GraphicalRepresentationSPtr rep);
      virtual bool managesRepresentation(GraphicalRepresentationSPtr rep);

      virtual IRendererSPtr clone()         { return IRendererSPtr(new SmoothedMeshRenderer()); }
  };

} /* namespace EspINA */

#endif /* SMOOTHEDMESHRENDERER_H_ */
