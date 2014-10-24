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

#ifndef ESPINA_SKELETON_REPRESENTATION_H_
#define ESPINA_SKELETON_REPRESENTATION_H_

// ESPINA
#include "Representation.h"
#include <Core/Analysis/Data/SkeletonData.h>
#include <GUI/ColorEngines/TransparencySelectionHighlighter.h>
#include <GUI/View/RenderView.h>
#include <GUI/View/View2D.h>
#include <GUI/View/View3D.h>

// VTK
#include <vtkSmartPointer.h>

class vtkPolyData;
class vtkPolyDataMapper;
class vtkActor;
class vtkImageCanvasSource2D;

namespace ESPINA
{
  class SkeletonRepresentation
  : public Representation
  {
    public:
      static const Representation::Type TYPE;

      /** \brief SkeletonRepresentation class constructor.
       * \param[in] data Smart pointer of the skeleton data to represent.
       * \param[in] view Smart pointer of the View where the representation is going to render.
       *
       */
      explicit SkeletonRepresentation(SkeletonDataSPtr data, RenderView *view);

      /** \brief SkeletonRepresentation class virtual destructor.
       *
       */
      virtual ~SkeletonRepresentation()
      {}

      virtual void setColor(const QColor &color) override;

      virtual void setHighlighted(bool highlighted) override;

      virtual bool isInside(const NmVector3 &point) const;

      virtual RepresentationSettings *settingsWidget();

      virtual RenderableView canRenderOnView() const
      { return RenderableView(Representation::RENDERABLEVIEW_SLICE|Representation::RENDERABLEVIEW_VOLUME); }

      virtual bool hasActor(vtkProp *actor) const;

      virtual QList<vtkProp *> getActors();

      virtual bool crosshairDependent() const
      { return false; }

      virtual bool needUpdate() const
      { return m_lastUpdatedTime != m_data->lastModified(); }

      virtual void updateRepresentation();

  protected:
      virtual RepresentationSPtr cloneImplementation(View3D *view)
      { return RepresentationSPtr{new SkeletonRepresentation{this->m_data, view}}; }

      virtual RepresentationSPtr cloneImplementation(View2D *view)
      { return RepresentationSPtr{new SkeletonRepresentation{this->m_data, view}}; }

      virtual void updateVisibility(bool visible);

    private:
      /** \brief Initializes the vtk pipeline with all the needed connections.
       *
       */
      virtual void initializePipeline();

      /** \brief Updates the actor texture with the current color and opacity.
       *
       */
      void updateTexture();

    protected:
      SkeletonDataSPtr                   m_data;
      Nm                                 m_slice;
      vtkSmartPointer<vtkPolyData>       m_polyData;
      vtkSmartPointer<vtkPolyDataMapper> m_mapper;
      vtkSmartPointer<vtkImageCanvasSource2D> m_textureIcon;
      vtkSmartPointer<vtkTexture>             m_texture;
      vtkSmartPointer<vtkActor>          m_actor;

      static TransparencySelectionHighlighter *s_highlighter;
  };

  using SkeletonRepresentationSPtr  = std::shared_ptr<SkeletonRepresentation>;
  using SkeletonRepresentationSList = QList<SkeletonRepresentationSPtr>;

} // namespace ESPINA

#endif // ESPINA_SKELETON_REPRESENTATION_H_
