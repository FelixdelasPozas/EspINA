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

#include "GUI/EspinaGUI_Export.h"

// ESPINA
#include "RepresentationRenderer.h"
#include <GUI/View/SelectableView.h>
#include <QFlags>

class vtkPropPicker;

namespace ESPINA
{
  class ViewManager;

  class EspinaGUI_EXPORT SliceRenderer
  : public RepresentationRenderer
  {
    public:
  		/* \brief SliceRenderer class constructor.
  		 * \param[in] parent, raw pointer of the QObject parent of this one.
  		 *
  		 */
      explicit SliceRenderer(QObject* parent = nullptr);

  		/* \brief SliceRenderer class virtual destructor.
  		 *
  		 */
      virtual ~SliceRenderer();

  		/* \brief Implements Renderer::icon() const.
  		 *
  		 */
      virtual const QIcon icon()      const   { return QIcon(":/espina/slice.png"); }

  		/* \brief Implements Renderer::name() const.
  		 *
  		 */
      virtual const QString name()    const   { return "Slice"; }

  		/* \brief Implements Renderer::tooltip() const.
  		 *
  		 */
      virtual const QString tooltip() const   { return "Segmentation's Slices"; }

  		/* \brief Implements RepresentationRenderer::addRepresentation().
  		 *
  		 */
      virtual void addRepresentation(ViewItemAdapterPtr item, RepresentationSPtr rep);

  		/* \brief Implements RepresentationRenderer::removeRepresentation().
  		 *
  		 */
      virtual void removeRepresentation(RepresentationSPtr rep);

  		/* \brief Implements RepresentationRenderer::hasRepresentation() const.
  		 *
  		 */
      virtual bool hasRepresentation(RepresentationSPtr rep) const;

  		/* \brief Implements RepresentationRenderer::managesRepresentation() const.
  		 *
  		 */
      virtual bool managesRepresentation(const QString &representationType) const;

  		/* \brief Implements Renderer::clone() cosnt.
  		 *
  		 */
      virtual RendererSPtr clone() const
      { return RendererSPtr(new SliceRenderer()); }

  		/* \brief Implements Renderer::numberOfvtkActors() const.
  		 *
  		 */
      virtual unsigned int numberOfvtkActors() const;

  		/* \brief Implements RepresentationRenderer::renderableItems() const.
  		 *
  		 */
      virtual RenderableItems renderableItems() const
      { return RenderableItems(RenderableType::CHANNEL|RenderableType::SEGMENTATION); }

  		/* \brief Implements RepresentationRenderer::renderType().
  		 *
  		 */
      virtual RendererTypes renderType() const
      { return RendererTypes(RENDERER_VIEW2D); }

  		/* \brief Implements Renderer::numberOfRenderedItems().
  		 *
  		 */
      virtual int numberOfRenderedItems() const
      { return m_representations.size(); }

  		/* \brief Implements RepresentationRenderer::pick().
  		 *
  		 */
      virtual ViewItemAdapterList pick(int x, int y, Nm z,
                                       vtkSmartPointer<vtkRenderer> renderer,
                                       RenderableItems itemType = RenderableItems(),
                                       bool repeat = false);

      /* \brief Overrides Renderer::setView().
       *
       */
      virtual void setView(RenderView *view) override;

      /* \brief Implements RepresentationRenderer::canRender() const.
       *
       */
      virtual bool canRender(ItemAdapterPtr item) const
      { return (item->type() == ItemAdapter::Type::CHANNEL || item->type() == ItemAdapter::Type::SEGMENTATION); }

    protected:
  		/* \brief Implements Renderer::hide().
  		 *
  		 */
      virtual void hide();

  		/* \brief Implements Renderer::show().
  		 *
  		 */
      virtual void show();

    private:
      vtkSmartPointer<vtkPropPicker> m_picker;
  };

} // namespace ESPINA
#endif // ESPINA_SLICE_RENDERER_H_
