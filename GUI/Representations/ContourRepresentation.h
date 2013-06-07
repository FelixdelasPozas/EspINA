/*
 <one line to give the program's name and a brief idea of what it does.>
 Copyright (C) 2013 Félix de las Pozas Álvarez <felixdelaspozas@gmail.com>

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

#ifndef CONTOURREPRESENTATION_H_
#define CONTOURREPRESENTATION_H_

// EspINA
#include <GUI/Representations/GraphicalRepresentation.h>
#include <Core/OutputRepresentations/VolumeRepresentation.h>
#include <GUI/QtWidget/SliceView.h>

// VTK
#include <vtkSmartPointer.h>

class vtkImageReslice;
class vtkPolyDataMapper;
class vtkActor;
class vtkVoxelContour2D;

namespace EspINA
{
  class TransparencySelectionHighlighter;
  class SliceView;
  class VolumeView;

  class ContourRepresentation
  : public SegmentationGraphicalRepresentation
  {
    Q_OBJECT
    public:
      ContourRepresentation(SegmentationVolumeSPtr data,
                            EspinaRenderView      *view);
      virtual ~ContourRepresentation() {};

      virtual void setColor(const QColor &color);

      virtual void setHighlighted(bool highlighted);

      virtual bool isInside(Nm point[3]);

      virtual RenderableView canRenderOnView() const
      { return GraphicalRepresentation::RenderableView(GraphicalRepresentation::RENDERABLEVIEW_SLICE); }

      virtual bool hasActor(vtkProp *actor) const;

      virtual void updateRepresentation();

      virtual QList<vtkProp*> getActors();

    protected:
      virtual GraphicalRepresentationSPtr cloneImplementation(SliceView *view);
      virtual GraphicalRepresentationSPtr cloneImplementation(VolumeView *view)
      { return GraphicalRepresentationSPtr(); }

      virtual void updateVisibility(bool visible);

    private slots:
      void updatePipelineConnections();

    private:
      void setView(SliceView *view) {m_view = view; };
      void initializePipeline();

    private:
      SegmentationVolumeSPtr m_data;

      vtkSmartPointer<vtkImageReslice>     m_reslice;
      vtkSmartPointer<vtkVoxelContour2D>   m_voxelContour;
      vtkSmartPointer<vtkPolyDataMapper>   m_mapper;
      vtkSmartPointer<vtkActor>            m_actor;
      static TransparencySelectionHighlighter *s_highlighter;
    };

    typedef boost::shared_ptr<ContourRepresentation> ContourRepresentationSPtr;
    typedef QList<ContourRepresentationSPtr> ContourRepresentationSList;

} /* namespace EspINA */
#endif /* CONTOURREPRESENTATION_H_ */
