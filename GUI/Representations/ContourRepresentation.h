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
#include <GUI/View/SliceView.h>

// VTK
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
  class TransparencySelectionHighlighter;
  class SliceView;
  class VolumeView;

  using SegmentationVolumeSPtr = std::shared_ptr<VolumetricData<itkVolumeType>>;

  class EspinaGUI_EXPORT ContourRepresentation
  : public Representation
  {
    Q_OBJECT
    public:
      typedef enum { tiny = 0, small, medium, large, huge } LineWidth;
      typedef enum { normal = 0, dotted, dashed } LinePattern;

      ContourRepresentation(SegmentationVolumeSPtr data,
                            RenderView      *view);
      virtual ~ContourRepresentation() {};

      virtual RepresentationSettings *settingsWidget();

      virtual void setColor(const QColor &color);

      virtual void setHighlighted(bool highlighted);

      virtual bool isInside(Nm point[3]);

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

    protected:
      virtual RepresentationSPtr cloneImplementation(SliceView *view);
      virtual RepresentationSPtr cloneImplementation(VolumeView *view)
      { return RepresentationSPtr(); }

      virtual void updateVisibility(bool visible);

    private slots:
      void updatePipelineConnections();

    private:
      void setView(SliceView *view) {m_view = view; };
      void initializePipeline();

    private:
      void generateTexture();
      SegmentationVolumeSPtr m_data;

      vtkSmartPointer<vtkImageReslice>         m_reslice;
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

    typedef boost::shared_ptr<ContourRepresentation> ContourRepresentationSPtr;
    typedef QList<ContourRepresentationSPtr> ContourRepresentationSList;

} /* namespace EspINA */
#endif /* CONTOURREPRESENTATION_H_ */
