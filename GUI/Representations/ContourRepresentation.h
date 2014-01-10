/*
 <one line to give the program's name and a brief idea of what it does.>
 Copyright (C) 2013 F�lix de las Pozas �lvarez <felixdelaspozas@gmail.com>

 This program is free software: you can redistribute it and/or modify
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

#include "EspinaGUI_Export.h"

// EspINA
#include "GUI/Representations/Representation.h"

#include <Core/Analysis/Data/VolumetricData.h>
#include <Core/Utils/NmVector3.h>

// ITK
#include <itkImageToVTKImageFilter.h>

// VTK
#include <vtkImageImport.h>
#include <vtkSmartPointer.h>
#include <vtkTubeFilter.h>

class vtkImageReslice;
class vtkPolyDataMapper;
class vtkActor;
class vtkVoxelContour2D;
class vtkImageCanvasSource2D;
class vtkTexture;

namespace EspINA
{
  class RepresentationSettings;
  class TransparencySelectionHighlighter;
  class View2D;
  class View3D;

  class EspinaGUI_EXPORT ContourRepresentation
  : public Representation
  {
    public:
      typedef enum { tiny = 0, small, medium, large, huge } LineWidth;
      typedef enum { normal = 0, dotted, dashed } LinePattern;

      ContourRepresentation(DefaultVolumetricDataSPtr data,
                            RenderView      *view);
      virtual ~ContourRepresentation() {};
      
      virtual RepresentationSettings *settingsWidget();

      virtual void setColor(const QColor &color);

      virtual void setHighlighted(bool highlighted);

      virtual bool isInside(const NmVector3& point) const;

      virtual RenderableView canRenderOnView() const
      { return Representation::RenderableView(Representation::RENDERABLEVIEW_SLICE); }

      virtual bool hasActor(vtkProp *actor) const;

      virtual void updateRepresentation();

      virtual QList<vtkProp*> getActors();

      void setLineWidth(LineWidth width);
      LineWidth lineWidth() const;

      void setLinePattern(LinePattern pattern);
      LinePattern linePattern() const;

      void updateWidth();
      void updatePattern();

      void setPlane(Plane plane)
      { m_planeIndex = normalCoordinateIndex(plane); }

      Plane plane()
      { return toPlane(m_planeIndex); }

      virtual bool crosshairDependent() const
      { return true; }

      virtual bool needUpdate()
      { return m_lastUpdatedTime != m_data->lastModified(); }

    protected:
      virtual RepresentationSPtr cloneImplementation(View2D *view);
      virtual RepresentationSPtr cloneImplementation(View3D *view)
      { return RepresentationSPtr(); }

      virtual void updateVisibility(bool visible);

    private:
      void setView(RenderView *view) { m_view = view; };
      void initializePipeline();

    private:
      void generateTexture();
      DefaultVolumetricDataSPtr m_data;
      int m_planeIndex;
      Nm m_reslicePoint;

      using ExporterType = itk::ImageToVTKImageFilter<itkVolumeType>;

      ExporterType::Pointer                    m_exporter;
      vtkSmartPointer<vtkImageImport>          m_importer;
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
    
} /* namespace EspINA */
#endif /* CONTOURREPRESENTATION_H_ */
