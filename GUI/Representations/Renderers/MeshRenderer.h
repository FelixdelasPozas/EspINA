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
#include "RepresentationRenderer.h"
#include "GUI/Representations/Representation.h"

class vtkPropPicker;

namespace EspINA
{
  class ViewManager;

  class EspinaGUI_EXPORT MeshRenderer
  : public RepresentationRenderer
  {
    public:
      explicit MeshRenderer(QObject* parent = 0);
      virtual ~MeshRenderer();

      virtual const QIcon icon()      const   { return QIcon(":/espina/mesh.png"); }
      virtual const QString name()    const   { return "Mesh"; }
      virtual const QString tooltip() const   { return "Segmentation's Meshes"; }

      virtual void addRepresentation(ViewItemAdapterPtr item, RepresentationSPtr rep);
      virtual void removeRepresentation(RepresentationSPtr rep);
      virtual bool hasRepresentation(RepresentationSPtr rep) const;
      virtual bool managesRepresentation(const QString &representationName) const;

      virtual RendererSPtr clone() const        { return RendererSPtr(new MeshRenderer()); }

      virtual unsigned int numberOfvtkActors() const;

      virtual RenderableItems renderableItems() const { return RenderableItems(EspINA::SEGMENTATION); }

      virtual RendererTypes renderType() const  { return RendererTypes(RENDERER_VIEW3D); }

      virtual int numberOfRenderedItems() const { return m_representations.size(); }

      virtual ViewItemAdapterList pick(int x, int y, Nm z,
                                       vtkSmartPointer<vtkRenderer> renderer,
                                       RenderableItems itemType = RenderableItems(),
                                       bool repeat = false);
    protected:
      virtual void hide();
      virtual void show();

      vtkSmartPointer<vtkPropPicker> m_picker;
  };

} // namespace EspINA

#endif // ESPINA_MESH_RENDERER_H
