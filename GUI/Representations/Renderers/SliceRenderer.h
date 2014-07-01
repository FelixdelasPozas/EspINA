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

#ifndef ESPINA_SLICE_RENDERER_H_
#define ESPINA_SLICE_RENDERER_H_

#include "EspinaGUI_Export.h"

// EspINA
#include "RepresentationRenderer.h"
#include <GUI/View/SelectableView.h>
#include <QFlags>

class vtkPropPicker;

namespace EspINA
{
  class ViewManager;
  
  class EspinaGUI_EXPORT SliceRenderer
  : public RepresentationRenderer
  {
    public:
      explicit SliceRenderer(QObject* parent = 0);
      virtual ~SliceRenderer();

      virtual const QIcon icon()      const   { return QIcon(":/espina/slice.png"); }
      virtual const QString name()    const   { return "Slice"; }
      virtual const QString tooltip() const   { return "Segmentation's Slices"; }

      virtual void addRepresentation(ViewItemAdapterPtr item, RepresentationSPtr rep);
      virtual void removeRepresentation(RepresentationSPtr rep);
      virtual bool hasRepresentation(RepresentationSPtr rep) const;
      virtual bool managesRepresentation(const QString &representationName) const;

      virtual RendererSPtr clone() const        { return RendererSPtr(new SliceRenderer()); }

      virtual unsigned int numberOfvtkActors() const;

      virtual RenderableItems renderableItems() const { return RenderableItems(RenderableType::CHANNEL|RenderableType::SEGMENTATION); }

      virtual RendererTypes renderType() const        { return RendererTypes(RENDERER_VIEW2D); }

      virtual int numberOfRenderedItems() const       { return m_representations.size(); }

      virtual ViewItemAdapterList pick(int x, int y, Nm z,
                                       vtkSmartPointer<vtkRenderer> renderer,
                                       RenderableItems itemType = RenderableItems(),
                                       bool repeat = false);

    protected:
      virtual void hide();
      virtual void show();

    private:
      vtkSmartPointer<vtkPropPicker> m_picker;
  };

} // namespace EspINA
#endif // ESPINA_SLICE_RENDERER_H_
