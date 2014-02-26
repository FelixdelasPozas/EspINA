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

#ifndef ESPINA_MESH_RENDERER_H
#define ESPINA_MESH_RENDERER_H

#include "EspinaGUI_Export.h"

// EspINA
#include "Renderer.h"
#include "GUI/Representations/Representation.h"

class vtkPropPicker;

namespace EspINA
{
  class ViewManager;

  class EspinaGUI_EXPORT MeshRenderer
  : public Renderer
  {
    public:
      explicit MeshRenderer(QObject* parent = 0);
      virtual ~MeshRenderer();

      virtual const QIcon icon()      const   { return QIcon(":/espina/mesh.png"); }
      virtual const QString name()    const   { return "Mesh"; }
      virtual const QString tooltip() const   { return "Segmentation's Meshes"; }

      virtual void addRepresentation(ViewItemAdapterPtr item, RepresentationSPtr rep);
      virtual void removeRepresentation(RepresentationSPtr rep);
      virtual bool hasRepresentation(RepresentationSPtr rep);
      virtual bool managesRepresentation(RepresentationSPtr rep);

      virtual RendererSPtr clone()              { return RendererSPtr(new MeshRenderer()); }

      virtual unsigned int numberOfvtkActors();

      virtual RenderableItems renderableItems() { return RenderableItems(EspINA::SEGMENTATION); }

      virtual RendererTypes renderType()        { return RendererTypes(RENDERER_VIEW3D); }

      virtual int numberOfRenderedItems()       { return m_representations.size(); }

      virtual ViewItemAdapterList pick(int x, int y, Nm z,
                                       vtkSmartPointer<vtkRenderer> renderer,
                                       RenderableItems itemType = RenderableItems(),
                                       bool repeat = false);

      virtual NmVector3 pickCoordinates() const;

    protected:
      virtual void hide();
      virtual void show();

      vtkSmartPointer<vtkPropPicker> m_picker;
  };

} // namespace EspINA

#endif // ESPINA_MESH_RENDERER_H
