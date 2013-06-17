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

#ifndef SLICERENDERER_H_
#define SLICERENDERER_H_

#include "EspinaGUI_Export.h"

// EspINA
#include "Renderer.h"

class vtkPropPicker;

namespace EspINA
{
  class ViewManager;
  
  class EspinaGUI_EXPORT SliceRenderer
  : public IRenderer
  {
    public:
      explicit SliceRenderer(QObject* parent = 0);
      virtual ~SliceRenderer();

      virtual const QIcon icon()      const   { return QIcon(":/espina/slice.png"); }
      virtual const QString name()    const   { return "Slice"; }
      virtual const QString tooltip() const   { return "Segmentation's Slices"; }

      virtual void addRepresentation(PickableItemPtr item, GraphicalRepresentationSPtr rep);
      virtual void removeRepresentation(GraphicalRepresentationSPtr rep);
      virtual bool hasRepresentation(GraphicalRepresentationSPtr rep);
      virtual bool managesRepresentation(GraphicalRepresentationSPtr rep);

      virtual void hide();
      virtual void show();

      virtual unsigned int getNumberOfvtkActors();

      virtual IRendererSPtr clone()                     { return IRendererSPtr(new SliceRenderer()); }

      virtual RendererType getRenderType()              { return RendererType(RENDERER_SLICEVIEW); }
      virtual RenderabledItems getRenderableItemsType() { return RenderabledItems(EspINA::CHANNEL|EspINA::SEGMENTATION); }
      virtual int itemsBeenRendered()                   { return m_representations.size(); }

      virtual ViewManager::Selection pick(int x,
                                          int y,
                                          Nm z,
                                          vtkSmartPointer<vtkRenderer> renderer,
                                          RenderabledItems itemType = RenderabledItems(),
                                          bool repeat = false);
      virtual void getPickCoordinates(Nm *point);

    private:
      vtkSmartPointer<vtkPropPicker> m_picker;

  };

} /* namespace EspINA */
#endif /* SLICERENDERER_H_ */
