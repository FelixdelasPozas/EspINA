/*

 Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>

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

#ifndef ESPINA_MESH_RENDERER_H
#define ESPINA_MESH_RENDERER_H

#include "GUI/EspinaGUI_Export.h"

// ESPINA
#include "RepresentationRenderer.h"
#include "GUI/Representations/Representation.h"

class vtkPropPicker;

namespace ESPINA
{
  class ViewManager;

  class EspinaGUI_EXPORT MeshRenderer
  : public RepresentationRenderer
  {
    public:
  		/* \brief MeshRenderer class constructor.
  		 * \param[in] parent, raw pointer of the QObject parent of this one.
  		 *
  		 */
      explicit MeshRenderer(QObject* parent = nullptr);

  		/* \brief MeshRenderer class virtual destructor.
  		 *
  		 */
      virtual ~MeshRenderer();

  		/* \brief Implements Renderer::icon() const.
  		 *
  		 */
      virtual const QIcon icon() const
      { return QIcon(":/espina/mesh.png"); }

  		/* \brief Implements Renderer::name() const.
  		 *
  		 */
      virtual const QString name() const
      { return "Mesh"; }

  		/* \brief Implements Renderer::tooltip() const.
  		 *
  		 */
      virtual const QString tooltip() const
      { return "Segmentation's Meshes"; }

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

  		/* \brief Implements RepresentationRenderer::managesRepresentation().
  		 *
  		 */
      virtual bool managesRepresentation(const QString &representationType) const;

  		/* \brief Implements Renderer::clone() const.
  		 *
  		 */
      virtual RendererSPtr clone() const
      { return RendererSPtr(new MeshRenderer()); }

  		/* \brief Implements Renderer::numberOfvtkActors() const.
  		 *
  		 */
      virtual unsigned int numberOfvtkActors() const;

  		/* \brief Implements RepresentationRenderer::renderableItems() const.
  		 *
  		 */
      virtual RenderableItems renderableItems() const
      { return RenderableItems(RenderableType::SEGMENTATION); }

  		/* \brief Implements RepresentationRenderer::renderType() const.
  		 *
  		 */
      virtual RendererTypes renderType() const
      { return RendererTypes(RENDERER_VIEW3D); }

  		/* \brief Implements Renderer::numberOfRenderedItems() const.
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

      /* \brief Overrides Renderer::setView(RenderView *view);
       *
       */
      virtual void setView(RenderView *view) override;

      /* \brief Implements RepresentationRenderer::canRender() const.
       *
       */
      virtual bool canRender(ItemAdapterPtr item) const
      { return (item->type() == ItemAdapter::Type::SEGMENTATION); }

    protected:
  		/* \brief Implements Renderer::hide().
  		 *
  		 */
      virtual void hide();

  		/* \brief Implements Renderer::show().
  		 *
  		 */
      virtual void show();

      vtkSmartPointer<vtkPropPicker> m_picker;
  };

} // namespace ESPINA

#endif // ESPINA_MESH_RENDERER_H
