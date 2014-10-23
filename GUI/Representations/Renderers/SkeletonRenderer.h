/*

 Copyright (C) 2014 Félix de las Pozas Álvarez <fpozas@cesvima.upm.es>

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

#ifndef ESPINA_SKELETON_RENDERER_H_
#define ESPINA_SKELETON_RENDERER_H_

#include "GUI/EspinaGUI_Export.h"

// ESPINA
#include "Renderer.h"
#include "RepresentationRenderer.h"

class vtkPropPicker;

namespace ESPINA
{
  class SkeletonRenderer
  : public RepresentationRenderer
  {
    public:
      /** \brief SkeletonRenderer class constructor.
       * \param[in] parent raw pointer of the QObject parent of this one.
       *
       */
      explicit SkeletonRenderer(QObject *parent);

      /** \brief SkeletonRenderer class virtual destructor.
       *
       */
      virtual ~SkeletonRenderer();

      virtual void setView(RenderView *view) override;

      virtual const QString name() const
      { return QString("Skeleton"); }

      virtual const QString tooltip() const
      { return QString("Segmentation's Skeleton"); }

      virtual const QIcon icon() const
      { return QIcon(":/espina/tubular.svg"); }

      virtual RendererSPtr clone() const
      { return RendererSPtr{new SkeletonRenderer(this->parent())}; }

      virtual RendererTypes renderType() const
      { return RendererTypes(RenderableType::SEGMENTATION); }

      virtual int numberOfRenderedItems() const
      { return m_representations.size(); }

      virtual bool canRender(ItemAdapterPtr item) const
      { return (item->type() == ItemAdapter::Type::SEGMENTATION); }

      virtual RenderableItems renderableItems() const
      { return RenderableItems(RenderableType::SEGMENTATION); }

      virtual unsigned int numberOfvtkActors() const;

      virtual void addRepresentation(ViewItemAdapterPtr item, RepresentationSPtr rep);

      virtual void removeRepresentation(RepresentationSPtr rep);

      virtual bool hasRepresentation(RepresentationSPtr rep) const;

      virtual bool managesRepresentation(const QString &representationType) const;

      virtual ViewItemAdapterList pick(int x, int y, Nm z, vtkSmartPointer<vtkRenderer> renderer, RenderableItems itemType = RenderableItems(), bool repeat = false);
    protected:
      virtual void show();
      virtual void hide();

      vtkSmartPointer<vtkPropPicker> m_picker;
  };

} // namespace ESPINA

#endif // ESPINA_SKELETON_RENDERER_H_
