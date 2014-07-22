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
#include <Core/Analysis/Data/VolumetricData.h>
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

  using VolumeSPtr = std::shared_ptr<VolumetricData<itkVolumeType>>;

  class EspinaGUI_EXPORT CrosshairRepresentation
  :public Representation
  {
    public:
      static const Representation::Type TYPE;

    public:
      explicit CrosshairRepresentation(VolumeSPtr data,
                                       RenderView *view);
      virtual ~CrosshairRepresentation() {};

      virtual void setBrightness(double value);

      virtual void setContrast(double value);

      virtual RepresentationSettings *settingsWidget();

      virtual void setColor(const QColor &color);

      virtual void setOpacity(double value);

      virtual bool isInside(const NmVector3 &point) const;

      virtual RenderableView canRenderOnView() const
      { return Representation::RENDERABLEVIEW_VOLUME; }

      virtual bool hasActor(vtkProp *actor) const;

      virtual void updateRepresentation();

      virtual QList<vtkProp*> getActors();

      virtual bool crosshairDependent() const
      { return true; }

      void setCrosshairColors(double axialColor[3], double coronalColor[3], double sagittalColor[3]);
      void setCrosshair(NmVector3 point);
      void setPlanePosition(Plane plane, Nm dist);

      bool tiling()              { return m_tiling; }
      void setTiling(bool value) { m_tiling = value; }

    protected:
      virtual RepresentationSPtr cloneImplementation(View2D *view)
      { return RepresentationSPtr(); }

      virtual RepresentationSPtr cloneImplementation(View3D *view);

    virtual void updateVisibility(bool visible);

    private:
      void initializePipeline();

    private:
      void setView(RenderView *view) { m_view = view; };

      VolumeSPtr m_data;

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
      bool m_tiling;

      friend class CrosshairRenderer;
  };

  typedef std::shared_ptr<CrosshairRepresentation> CrosshairRepresentationSPtr;
  typedef QList<CrosshairRepresentationSPtr> CrosshairRepresentationSList;

} // namespace ESPINA

#endif // ESPINA_CROSSHAIR_REPRESENTATION_H
