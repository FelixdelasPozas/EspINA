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

#ifndef ESPINA_REPRESENTATION_RENDERER_H_
#define ESPINA_REPRESENTATION_RENDERER_H_

#include "GUI/EspinaGUI_Export.h"

// ESPINA
#include "Renderer.h"
#include <GUI/Representations/Representation.h>
#include <GUI/Model/ViewItemAdapter.h>

namespace ESPINA
{
  enum RenderableType
  {
    CHANNEL      = 0x1,
    SEGMENTATION = 0x2,
  };
  Q_DECLARE_FLAGS(RenderableItems, RenderableType);

  class EspinaGUI_EXPORT RepresentationRenderer
  : public Renderer
  {
    public:
  		/* \brief RepresentationRenderer class constructor.
  		 * \param[in] parent, raw pointer of the QObject parent of this one.
  		 *
  		 */
      explicit RepresentationRenderer(QObject* parent = nullptr)
      : Renderer{parent}
      {};

      /* \brief RepresentationRenderer class virtual destructor.
       *
       */
      virtual ~RepresentationRenderer()
      {};

      /* \brief Adds a representation to the renderer and assigns it to the specified ViewItemAdapter.
       * \param[in] item, smart pointer of the view item adapter that the representation belongs to.
       * \param[in] rep, representation smart pointer.
       *
       */
      virtual void addRepresentation(ViewItemAdapterPtr item, RepresentationSPtr rep) = 0;

      /* \brief Removes a representation from the renderer.
       * \param[in] rep, representation smart pointer.
       *
       */
      virtual void removeRepresentation(RepresentationSPtr rep) = 0;

      /* \brief Returns true if the renderer manages the representation.
       *
       */
      virtual bool hasRepresentation(RepresentationSPtr rep) const = 0;

      /* \brief Returns true if the renderer can manage the specified type of representations.
       * \param[in] representationType, representation type.
       *
       */
      virtual bool managesRepresentation(const QString &representationType) const = 0;

      /* \brief Returns the type of items the renderer can manage representations of.
       *
       */
      virtual RenderableItems renderableItems() const = 0;

      /* \brief Returns true if the renderer can manage a representation for the specified item.
       * \param[in] item, item adapter smart pointer.
       *
       * TODO: naive item filtering, to be modified/enhanced in the future.
       *
       */
      virtual bool canRender(ItemAdapterPtr item) const = 0;

      /* \brief Returns the list of items picked by the specified coordinates, type and belonging to the renderer specified.
       * \param[in] x, x display coordinate.
       * \param[in] y, y display coordinate.
       * \param[in] z, z work coordinate (depth)
       * \param[in] renderer, smart pointer of the vtkRenderer to perform the picking.
       * \param[in] itemType, flags of types of items to pick.
       * \param[in] repeat, true to return a list of all the items picked, false to return only the first one.
       *
       */
      virtual ViewItemAdapterList pick(int x, int y, Nm z, vtkSmartPointer<vtkRenderer> renderer, RenderableItems itemType = RenderableItems(), bool repeat = false) = 0;

      /* \brief Returns the world coodinates of a successful pick.
       *
       */
      virtual NmVector3 pickCoordinates()
      { return m_lastValidPickPosition; };

      /* \brief Implements Renderer::type()
       *
       */
      virtual Type type() const
      { return Type::Representation; }

    protected:
      QMap<ViewItemAdapterPtr, RepresentationSList> m_representations;
      NmVector3                                     m_lastValidPickPosition;

  };

  class EspinaGUI_EXPORT ChannelRenderer
  : public RepresentationRenderer
  {
    public:
      /* \brief ChannelRenderer class constructor.
  		 * \param[in] parent, raw pointer of the QObject parent of this one.
       *
       */
      explicit ChannelRenderer(QObject* parent = 0)
      : RepresentationRenderer(parent)
      {}

      /* \brief ChannelRenderer class destructor.
       *
       */
      virtual ~ChannelRenderer()
      {}

      /* \brief Sets the colors of the crosshairs.
       * \param[in] axialColor, vector of doubles with the r,g,b components of the color.
       * \param[in] coronalColor, vector of doubles with the r,g,b components of the color.
       * \param[in] sagittalColor, vector of doubles with the r,g,b components of the color.
       *
       */
      virtual void setCrosshairColors(double axialColor[3], double coronalColor[3], double sagittalColor[3]) = 0;

      /* \brief Sets the crosshair position in the channel renderer.
       * \param[in] point, crosshair point.
       *
       */
      virtual void setCrosshair(NmVector3 point) = 0;

      /* \brief Moves one of the planes of the crosshair to the specified position.
       * \param[in] plane, plane to move.
       * \param[in] pos, new plane position.
       *
       */
      virtual void setPlanePosition(Plane plane, Nm pos) = 0;

      /* \brief Implements RepresentationRenderer::renderableItems().
       *
       */
      virtual RenderableItems renderableItems() const
      { return RenderableItems(RenderableType::CHANNEL); }
  };

  using RepresentationRendererPtr   = RepresentationRenderer *;
  using RepresentationRendererSPtr  = std::shared_ptr<RepresentationRenderer>;
  using RepresentationRendererList  = QList<RepresentationRendererPtr>;
  using RepresentationRendererSList = QList<RepresentationRendererSPtr>;

  /* \brief Returns true if the renderer can render the specified type of items.
   *
   */
  bool canRender(RepresentationRendererSPtr renderer, RenderableType type);

  /* \brief Returns the representation renderer smart pointer of the specified renderer.
   * \param[in] renderer, renderer smart pointer.
   *
   */
  RepresentationRendererSPtr representationRenderer(RendererSPtr renderer);

  Q_DECLARE_OPERATORS_FOR_FLAGS(RenderableItems)

} // namespace ESPINA

#endif // ESPINA_REPRESENTATION_RENDERER_H_
