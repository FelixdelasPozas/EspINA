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

#ifndef ESPINA_MESH_REPRESENTATION_BASE_H
#define ESPINA_MESH_REPRESENTATION_BASE_H

#include "GUI/EspinaGUI_Export.h"

// ESPINA
#include "Representation.h"
#include <Core/Analysis/Data/MeshData.h>
#include <GUI/View/View3D.h>

// VTK
#include <vtkSmartPointer.h>

class vtkPolyDataMapper;
class vtkActor;

namespace ESPINA
{
  class TransparencySelectionHighlighter;

  class EspinaGUI_EXPORT MeshRepresentationBase
  : public Representation
  {
    Q_OBJECT
    public:
			/** \brief MeshRepresentationBase class constructor.
			 * \param[in] mesh, MeshData smart pointer of the data to represent.
			 * \param[in] view, render view pointer where the representation will be shown.
			 *
			 */
      explicit MeshRepresentationBase(MeshDataSPtr data,
                                      RenderView *view);

			/** \brief MeshRepresentationBase class virtual destructor.
			 *
			 */
      virtual ~MeshRepresentationBase()
      {};

			/** \brief Overrides Representation::setColor().
			 *
			 */
      virtual void setColor(const QColor &color) override;

			/** \brief Overrides Representation::setHighlighted().
			 *
			 */
      virtual void setHighlighted(bool highlighted) override;

			/** \brief Implements Representation::isInside() const.
			 *
			 */
      virtual bool isInside(const NmVector3 &point) const;

			/** \brief Implements Representation::canRenderOnView() const.
			 *
			 */
      virtual RenderableView canRenderOnView() const
      { return Representation::RENDERABLEVIEW_VOLUME; }

			/** \brief Implements Representation::hasActor() const.
			 *
			 */
      virtual bool hasActor(vtkProp *actor) const;

			/** \brief Implements Representation::getActors().
			 *
			 */
      virtual QList<vtkProp *> getActors();

      /** \brief Implements Representation::crosshairDependent().
       *
       */
      virtual bool crosshairDependent() const
      { return false; }

      /** \brief Implements Representation::needUpdate().
       *
       */
      virtual bool needUpdate() const
      { return m_lastUpdatedTime != m_data->lastModified(); }

  protected:
			/** \brief Implements Representation::cloneImplementation(View2D*).
			 *
			 */
      virtual RepresentationSPtr cloneImplementation(View2D *view)
      { return RepresentationSPtr(); }

			/** \brief Implements Representation::updateVisibility().
			 *
			 */
      virtual void updateVisibility(bool visible);

			/** \brief Sets the view of the representation.
			 * \param[in] view, renderview smart pointer.
			 *
			 */
      virtual void setView(RenderView *view)
      { m_view = view; };

    private:
			/** \brief Helper method to initialize the vtk pipeline.
			 *
			 */
      virtual void initializePipeline() = 0;

    protected:
      MeshDataSPtr m_data;
      vtkSmartPointer<vtkPolyDataMapper> m_mapper;
      vtkSmartPointer<vtkActor>          m_actor;

      static TransparencySelectionHighlighter *s_highlighter;
  };

  // one shouldn't address objects of this class directly but use the subclasses,
  // however this is here for convenience.
  using MeshRepresentationBaseSPtr  = std::shared_ptr<MeshRepresentationBase>;
  using MeshRepresentationBaseSList = QList<MeshRepresentationBaseSPtr>;

} // namespace ESPINA
#endif // ESPINA_MESH_REPRESENTATION_BASE_H
