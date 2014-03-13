/*
 <one line to give the program's name and a brief idea of what it does.>
 Copyright (C) 2014 Félix de las Pozas Álvarez <felixdelaspozas@gmail.com>

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

#ifndef ESPINA_REPRESENTATION_RENDERER_H_
#define ESPINA_REPRESENTATION_RENDERER_H_

// EspINA
#include "Renderer.h"
#include <GUI/Representations/Representation.h>
#include <GUI/Model/ViewItemAdapter.h>

namespace EspINA
{
  enum RenderableType
  {
    CHANNEL      = 0x1,
    SEGMENTATION = 0x2,
  };
  Q_DECLARE_FLAGS(RenderableItems, RenderableType);
  
  class RepresentationRenderer
  : public Renderer
  {
    public:
      explicit RepresentationRenderer(QObject* parent = nullptr)
      : Renderer(parent)
      {};

      virtual ~RepresentationRenderer()
      {};

      virtual void addRepresentation(ViewItemAdapterPtr item, RepresentationSPtr rep) = 0;
      virtual void removeRepresentation(RepresentationSPtr rep) = 0;
      virtual bool hasRepresentation(RepresentationSPtr rep) const = 0;
      virtual bool managesRepresentation(const QString &representationName) const = 0;

      virtual RenderableItems renderableItems() const
      { return RenderableItems(); }

      // naive item filtering, to be modified/enhanced in the future
      virtual bool canRender(ItemAdapterPtr item) const
      { return true; }

      virtual ViewItemAdapterList pick(int x, int y, Nm z, vtkSmartPointer<vtkRenderer> renderer, RenderableItems itemType = RenderableItems(), bool repeat = false) = 0;

      virtual NmVector3 pickCoordinates() const = 0;

      virtual Type type() const
      { return Type::Representation; }

    protected:
      QMap<ViewItemAdapterPtr, RepresentationSList> m_representations;
  };

  class EspinaGUI_EXPORT ChannelRenderer
  : public RepresentationRenderer
  {
    public:
      explicit ChannelRenderer(QObject* parent = 0)
      : RepresentationRenderer(parent)
      {}

      virtual ~ChannelRenderer()
      {}

      virtual void setCrosshairColors(double axialColor[3], double coronalColor[3], double sagittalColor[3]) = 0;
      virtual void setCrosshair(NmVector3 point) = 0;
      virtual void setPlanePosition(Plane plane, Nm dist) = 0;
  };

  using RepresentationRendererPtr   = RepresentationRenderer *;
  using RepresentationRendererSPtr  = std::shared_ptr<RepresentationRenderer>;
  using RepresentationRendererList  = QList<RepresentationRendererPtr>;
  using RepresentationRendererSList = QList<RepresentationRendererSPtr>;

  bool canRender(RepresentationRendererSPtr renderer, RenderableType type);

  RepresentationRendererSPtr representationRenderer(RendererSPtr renderer);

  Q_DECLARE_OPERATORS_FOR_FLAGS(RenderableItems)

} // namespace EspINA

#endif // ESPINA_REPRESENTATION_RENDERER_H_
