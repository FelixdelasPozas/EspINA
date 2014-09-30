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

#ifndef ESPINA_CONTOUR_RENDERER_H_
#define ESPINA_CONTOUR_RENDERER_H_

#include "GUI/EspinaGUI_Export.h"

// ESPINA
#include "MeshRenderer.h"

namespace ESPINA
{
  class EspinaGUI_EXPORT ContourRenderer
  : public MeshRenderer
  {
    public:
  		/** \brief ContourRenderer class constructor.
  		 * \param[in] parent, raw pointer of the QObject parent of this one.
  		 *
  		 */
      ContourRenderer(QObject* parent = nullptr);

  		/** \brief ContourRenderer class virtual destructor.
  		 *
  		 */
      virtual ~ContourRenderer();

  		/** \brief Overrides MeshRenderer::icon() const.
  		 *
  		 */
      virtual const QIcon icon() const override
      { return QIcon(":/espina/contour.png"); }

      /** \brief Overrides MeshRenderer::name() const.
  		 *
  		 */
      virtual const QString name() const override
      { return "Contour"; }

      /** \brief Overrides MeshRenderer::tooltip() const.
  		 *
  		 */
      virtual const QString tooltip() const override
      { return "Segmentation's Contours"; }

  		/** \brief Overrides MeshRenderer::addRepresentation().
  		 *
  		 */
      virtual void addRepresentation(ViewItemAdapterPtr item, RepresentationSPtr rep) override;

  		/** \brief Overrides MeshRenderer::removeRepresentation().
  		 *
  		 */
      virtual void removeRepresentation(RepresentationSPtr rep) override;

  		/** \brief Overrides MeshRenderer::managesRepresentation() const.
  		 *
  		 */
      virtual bool managesRepresentation(const QString &representationType) const override;

  		/** \brief Overrides MeshRenderer::clone() const.
  		 *
  		 */
      virtual RendererSPtr clone() const override
      { return RendererSPtr(new ContourRenderer()); }

  		/** \brief Overrride MeshRenderer::renderType() const.
  		 *
  		 */
      virtual RendererTypes renderType() const override
      { return RendererTypes(RENDERER_VIEW2D); }

  		/** \brief Overrides MeshRenderer::pick()
  		 *
  		 */
      virtual ViewItemAdapterList pick(int x, int y, Nm z,
                                       vtkSmartPointer<vtkRenderer> renderer,
                                       RenderableItems itemType = RenderableItems(),
                                       bool repeat = false) override;
  };

} // namespace ESPINA
#endif // ESPINA_CONTOUR_RENDERER_H_
