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

#ifndef MESHRENDERER_H
#define MESHRENDERER_H

#include "EspinaGUI_Export.h"

// EspINA
#include "Renderer.h"
#include "GUI/Representations/GraphicalRepresentation.h"
#include <Core/Model/Output.h>
#include <GUI/ViewManager.h>
#include <Core/EspinaTypes.h>

// VTK
#include <vtkSmartPointer.h>

class vtkPropPicker;

namespace EspINA
{
  class ViewManager;

  class EspinaGUI_EXPORT MeshRenderer
  : public IRenderer
  {
    public:
      explicit MeshRenderer(QObject* parent = 0);
      virtual ~MeshRenderer();

      virtual const QIcon icon()      const   { return QIcon(":/espina/mesh.png"); }
      virtual const QString name()    const   { return "Mesh"; }
      virtual const QString tooltip() const   { return "Segmentation's Meshes"; }

      virtual void addRepresentation(PickableItemPtr item, GraphicalRepresentationSPtr rep);
      virtual void removeRepresentation(GraphicalRepresentationSPtr rep);
      virtual bool hasRepresentation(GraphicalRepresentationSPtr rep);
      virtual bool managesRepresentation(GraphicalRepresentationSPtr rep);

      virtual void hide();
      virtual void show();

      virtual unsigned int getNumberOfvtkActors();

      virtual IRendererSPtr clone()                     { return IRendererSPtr(new MeshRenderer()); }
      virtual RendererType getRenderType()              { return RendererType(RENDERER_VOLUMEVIEW); }
      virtual RenderabledItems getRenderableItemsType() { return RenderabledItems(EspINA::SEGMENTATION); }
      virtual int itemsBeenRendered()                   { return m_representations.size(); }

      virtual ViewManager::Selection pick(int x,
                                          int y,
                                          Nm z,
                                          vtkSmartPointer<vtkRenderer> renderer,
                                          RenderabledItems itemType = RenderabledItems(),
                                          bool repeat = false);
      virtual void getPickCoordinates(Nm *point);

    protected:
      vtkSmartPointer<vtkPropPicker> m_picker;
  };

} // namespace EspINA

#endif // MESHRENDERER_H
