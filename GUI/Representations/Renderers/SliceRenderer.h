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

#ifndef SLICERENDERER_H_
#define SLICERENDERER_H_

#include "EspinaGUI_Export.h"

// EspINA
#include "Renderer.h"
#include <GUI/View/SelectableView.h>
#include <QFlags>

class vtkPropPicker;

namespace EspINA
{
  class ViewManager;
  
  class EspinaGUI_EXPORT SliceRenderer
  : public Renderer
  {
    public:
      explicit SliceRenderer(QObject* parent = 0);
      virtual ~SliceRenderer();

      virtual const QIcon icon()      const   { return QIcon(":/espina/slice.png"); }
      virtual const QString name()    const   { return "Slice"; }
      virtual const QString tooltip() const   { return "Segmentation's Slices"; }

      virtual void addRepresentation(ViewItemAdapterPtr item, RepresentationSPtr rep);
      virtual void removeRepresentation(RepresentationSPtr rep);
      virtual bool hasRepresentation(RepresentationSPtr rep);
      virtual bool managesRepresentation(RepresentationSPtr rep);

      virtual void hide();
      virtual void show();

      virtual RendererSPtr clone()              { return RendererSPtr(new SliceRenderer()); }

      virtual unsigned int numberOfvtkActors();

      virtual RenderableItems renderableItems()  { return RenderableItems(EspINA::CHANNEL|EspINA::SEGMENTATION); }

      virtual RendererTypes renderType()         { return RendererTypes(RENDERER_SLICEVIEW); }

      virtual bool canRender(ItemAdapterPtr item);

      virtual int numberOfRenderedItems()       { return m_representations.size(); }

      virtual SelectableView::Selection pick(int x,
                                             int y,
                                             Nm z,
                                             vtkSmartPointer<vtkRenderer> renderer,
                                             RenderableItems itemType = RenderableItems(),
                                             bool repeat = false);

      virtual NmVector3 pickCoordinates() const;

    private:
      vtkSmartPointer<vtkPropPicker> m_picker;
  };

} /* namespace EspINA */
#endif /* SLICERENDERER_H_ */
