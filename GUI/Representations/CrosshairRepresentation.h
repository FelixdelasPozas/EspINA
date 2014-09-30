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

#ifndef ESPINA_CROSSHAIR_REPRESENTATION_H
#define ESPINA_CROSSHAIR_REPRESENTATION_H

#include "GUI/EspinaGUI_Export.h"

// ESPINA
#include "Representation.h"
#include <Core/Analysis/Data/VolumetricData.hxx>
#include <Core/Utils/Spatial.h>
#include <GUI/View/RenderView.h>

// VTK
#include <vtkSmartPointer.h>

// ITK
#include <itkImageToVTKImageFilter.h>

class QColor;
class vtkActor;
class vtkImageActor;
class vtkPolyData;
class vtkLookupTable;
class vtkImageShiftScale;
class vtkImageMapToColors;

namespace ESPINA
{
  class View2D;
  class View3D;
  class RenderView;
  class CrosshairRenderer;

  class EspinaGUI_EXPORT CrosshairRepresentation
  :public Representation
  {
    public:
      static const Representation::Type TYPE;

    public:
      /** \brief CrosshairRepresentation class constructor.
       * \param[in] data, volumetric data smart pointer of the data to represent.
       * \param[in] view, renderview raw pointer the representation will be shown.
       *
       */
      explicit CrosshairRepresentation(DefaultVolumetricDataSPtr data,
                                       RenderView *view);

      /** \brief CrosshairRepresentation class virtual destructor.
       *
       */
      virtual ~CrosshairRepresentation()
      {};

      /** \brief Implements Representation::settingsWidget().
       *
       */
      virtual RepresentationSettings *settingsWidget();

      /** \brief Overrides Representation::setBrightness().
       *
       */
      virtual void setBrightness(double value) override;

      /** \brief Overrides Representation::setContrast().
       *
       */
      virtual void setContrast(double value) override;

      /** \brief Overrides Representation::setColor()
       *
       */
      virtual void setColor(const QColor &color) override;

      /** \brief Overrides Representation::setOpacity().
       *
       */
      virtual void setOpacity(double value) override;

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

      /** \brief Implements Representation::updateRepresentation().
       *
       */
      virtual void updateRepresentation();

      /** \brief Implements Representation::getActors().
       *
       */
      virtual QList<vtkProp*> getActors();

      /** \brief Implements Representation::crosshairDependent() const.
       *
       */
      virtual bool crosshairDependent() const
      { return true; }

      /** \brief Implements Representation::needUpdate().
       *
       */
      virtual bool needUpdate() const
      { return m_lastUpdatedTime != m_data->lastModified(); }

      /** \brief Set the colors of the representation crosshair.
       * \param[in] axialColor, vector of doubles with the r,g,b components of the color.
       * \param[in] coronalColor, vector of doubles with the r,g,b components of the color.
       * \param[in] sagittalColor, vector of doubles with the r,g,b components of the color.
       *
       */
      void setCrosshairColors(double axialColor[3], double coronalColor[3], double sagittalColor[3]);

      /** \brief Sets the crosshair position in the representation.
       * \param[in] point, crosshair point.
       */
      void setCrosshair(NmVector3 point);

      /** \brief Moves one of the planes of the crosshair to the specified position.
       * \param[in] plane, plane to move.
       * \param[in] pos, new position.
       */
      void setPlanePosition(Plane plane, Nm pos);

      /** \brief Returns true if the plane is a tile.
       *
       */
      bool tiling()
      { return m_tiling; }

      /** \brief Sets the representation as part of the tiling procedure.
       * \param[in] value, true to set as a tile false otherwise.
       *
       */
      void setTiling(bool value)
      { m_tiling = value; }

    protected:
      /** \brief Implements Representation::cloneImplementation(View2D*).
       *
       */
      virtual RepresentationSPtr cloneImplementation(View2D *view)
      { return RepresentationSPtr(); }

      /** \brief Implements Representation::cloneImplementation(View3D*).
       *
       */
      virtual RepresentationSPtr cloneImplementation(View3D *view);

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

      /** \brief Helper method to initilize the vtk pipeline.
       *
       */
      void initializePipeline();

      DefaultVolumetricDataSPtr m_data;

      using ExporterType = itk::ImageToVTKImageFilter<itkVolumeType>;

      ExporterType::Pointer                m_axialExporter;
      ExporterType::Pointer                m_coronalExporter;
      ExporterType::Pointer                m_sagittalExporter;
      vtkSmartPointer<vtkImageActor>       m_axial;
      vtkSmartPointer<vtkImageActor>       m_coronal;
      vtkSmartPointer<vtkImageActor>       m_sagittal;
      vtkSmartPointer<vtkImageMapToColors> m_axialImageMapToColors;
      vtkSmartPointer<vtkImageMapToColors> m_coronalImageMapToColors;
      vtkSmartPointer<vtkImageMapToColors> m_sagittalImageMapToColors;
      vtkSmartPointer<vtkActor>            m_axialBorder;
      vtkSmartPointer<vtkActor>            m_coronalBorder;
      vtkSmartPointer<vtkActor>            m_sagittalBorder;
      vtkSmartPointer<vtkPolyData>         m_axialSquare;
      vtkSmartPointer<vtkPolyData>         m_coronalSquare;
      vtkSmartPointer<vtkPolyData>         m_sagittalSquare;
      vtkSmartPointer<vtkLookupTable>      m_lut;
      vtkSmartPointer<vtkImageShiftScale>  m_axialScaler;
      vtkSmartPointer<vtkImageShiftScale>  m_coronalScaler;
      vtkSmartPointer<vtkImageShiftScale>  m_sagittalScaler;

      mutable Bounds m_bounds;
      NmVector3 m_point;
      NmVector3 m_lastUpdatePoint;
      bool m_tiling;

      friend class CrosshairRenderer;
  };

  typedef std::shared_ptr<CrosshairRepresentation> CrosshairRepresentationSPtr;
  typedef QList<CrosshairRepresentationSPtr> CrosshairRepresentationSList;

} // namespace ESPINA

#endif // ESPINA_CROSSHAIR_REPRESENTATION_H
