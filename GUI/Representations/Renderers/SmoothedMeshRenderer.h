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

#ifndef ESPINA_SMOOTHED_MESH_RENDERER_H_
#define ESPINA_SMOOTHED_MESH_RENDERER_H_

#include "GUI/EspinaGUI_Export.h"

// ESPINA
#include "MeshRenderer.h"

namespace ESPINA
{
  class ViewManager;

  class EspinaGUI_EXPORT SmoothedMeshRenderer
  : public MeshRenderer
  {
    public:
  		/** \brief SmoothedMeshRenderer class constructor.
  		 * \param[in] parent, raw pointer of the QObject parent of this one.
  		 *
  		 */
      explicit SmoothedMeshRenderer(QObject* parent = nullptr);

      /** \brief SmoothedMeshRenderer class virtual destructor.
       *
       */
      virtual ~SmoothedMeshRenderer()
      {}

      /** \brief Implements Renderer::icon() const.
       *
       */
      virtual const QIcon icon() const
      { return QIcon(":/espina/smoothedmesh.png"); }

      /** \brief Implements Renderer::name() const.
       *
       */
      virtual const QString name()
      const { return "Smoothed Mesh"; }

      /** \brief Implements Renderer::tooltip() const.
       *
       */
      virtual const QString tooltip() const
      { return "Segmentation's Smoothed Meshes"; }

      /** \brief Implements RepresentationRenderer::addRepresentation().
       *
       */
      virtual void addRepresentation(ViewItemAdapterPtr item, RepresentationSPtr rep);

      /** \brief Implements RepresentationRenderer::removeRepresentation().
       *
       */
      virtual void removeRepresentation(RepresentationSPtr rep);

      /** \brief Implements RepresentationRenderer::managesRepresentation() const.
       *
       */
      virtual bool managesRepresentation(const QString &repType) const;

      /** \brief Implements Renderer::clone() const.
       *
       */
      virtual RendererSPtr clone() const
      { return RendererSPtr(new SmoothedMeshRenderer()); }
  };

} /* namespace ESPINA */

#endif /* ESPINA_SMOOTHED_MESH_RENDERER_H_ */
