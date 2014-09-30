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

#ifndef ESPINA_CONTOUR_REPRESENTATION_H
#define ESPINA_CONTOUR_REPRESENTATION_H

#include "GUI/EspinaGUI_Export.h"

// ESPINA
#include <Core/Analysis/Data/VolumetricData.hxx>
#include <Core/Utils/NmVector3.h>
#include "GUI/Representations/Representation.h"

// VTK
#include <vtkSmartPointer.h>
#include <vtkTubeFilter.h>

class vtkImageReslice;
class vtkPolyDataMapper;
class vtkActor;
class vtkVoxelContour2D;
class vtkImageCanvasSource2D;
class vtkTexture;

namespace ESPINA
{
  class RepresentationSettings;
  class TransparencySelectionHighlighter;
  class View2D;
  class View3D;

  class EspinaGUI_EXPORT ContourRepresentation
  : public Representation
  {
    public:
      enum class LineWidth: std::int8_t { tiny = 0, small = 1, medium = 2, large = 3, huge = 4 };
      enum class LinePattern: std::int8_t { normal = 0, dotted = 1, dashed = 2 } ;

      static const Representation::Type TYPE;

      /** \brief ContourRepresentation class constructor.
       * \param[in] data, volumetric data smart pointer of the data to represent.
       * \param[in] view, renderview raw pointer the representation will be shown.
       *
       */
      ContourRepresentation(DefaultVolumetricDataSPtr data,
                            RenderView      *view);

      /** \brief ContourRepresentation class virtual destructor.
       *
       */
      virtual ~ContourRepresentation()
      {};

      /** \brief Implements Representation::settingsWidget().
       *
       */
      virtual RepresentationSettings *settingsWidget();

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
      virtual bool isInside(const NmVector3& point) const;

      /** \brief Implements Representation::canRenderViewOn() const.
       *
       */
      virtual RenderableView canRenderOnView() const
      { return Representation::RenderableView(Representation::RENDERABLEVIEW_SLICE); }

      /** \brief Implements Representation::hasActor() const.
       *
       */
      virtual bool hasActor(vtkProp *actor) const;

      /** \brief Implements Representation::updateRepresentation().
       *
       */
      virtual void updateRepresentation();

      /** \brief Implements Representation::getActors().
       *
       */
      virtual QList<vtkProp*> getActors();

      /** \brief Sets the with of the representation.
       * \param[in] width, LineWidth value.
       *
       */
      void setLineWidth(LineWidth width);

      /** \brief Returns the width of the representation.
       *
       */
      LineWidth lineWidth() const;

      /** \brief Sets the line pattern of the representation.
       * \param[in] pattern, LinePattern value.
       *
       */
      void setLinePattern(LinePattern pattern);
      LinePattern linePattern() const;

      /** \brief Updates the width of the representation().
       *
       */
      void updateWidth();

      /** \brief Updates the pattern of the representation().
       *
       */
      void updatePattern();

      /** \brief Sets the plane of the contour representation.
       * \param[in] plane, plane of the representation.
       *
       */
      void setPlane(Plane plane)
      { m_planeIndex = normalCoordinateIndex(plane); }

      /** \brief Returns the plane of the representation().
       *
       */
      Plane plane()
      { return toPlane(m_planeIndex); }

      /** \brief Implements Representation::crosshairDependent() const.
       *
       */
      virtual bool crosshairDependent() const
      { return true; }

      /** \brief Implements Representation::needUpdate() const.
       *
       */
      virtual bool needUpdate() const
      { return m_lastUpdatedTime != m_data->lastModified(); }

    protected:
      /** \brief Implements Representation::cloneImplementation(View2D*).
       *
       */
      virtual RepresentationSPtr cloneImplementation(View2D *view);

      /** \brief Implements Representation::cloneImplementation(View3D*).
       *
       */
      virtual RepresentationSPtr cloneImplementation(View3D *view)
      { return RepresentationSPtr(); }

      /** \brief Implements Representation::updateVisibility().
       *
       */
      virtual void updateVisibility(bool visible);

    private:
      /** \brief Helper method to set the view of the representation.
       * \param[in] view, RenderView raw pointer.
       *
       */
      void setView(RenderView *view)
      { m_view = view; };

      /** \brief Helper method to initialize the vtk pipeline.
       *
       */
      void initializePipeline();

    private:
      /** \brief Helper method to generate the texture for the line pattern.
       *
       */
      void generateTexture();
      int m_planeIndex;
      Nm  m_reslicePoint;
      DefaultVolumetricDataSPtr                m_data;
      vtkSmartPointer<vtkVoxelContour2D>       m_voxelContour;
      vtkSmartPointer<vtkImageCanvasSource2D>  m_textureIcon;
      vtkSmartPointer<vtkTexture>              m_texture;
      vtkSmartPointer<vtkTubeFilter>           m_tubes;
      vtkSmartPointer<vtkPolyDataMapper>       m_mapper;
      vtkSmartPointer<vtkActor>                m_actor;
      static TransparencySelectionHighlighter *s_highlighter;

      LineWidth m_width;
      LinePattern m_pattern;
      Nm m_minSpacing;
    };

    using ContourRepresentationPtr  = ContourRepresentation *;
    using ContourRepresentationSPtr = std::shared_ptr<ContourRepresentation>;
    using ContourRepresentationSList = QList<ContourRepresentationSPtr>;

} /* namespace ESPINA */
#endif /* CONTOURREPRESENTATION_H_ */
