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

#ifndef MESHREPRESENTATION_H_
#define MESHREPRESENTATION_H_

// EspINA
#include <GUI/Representations/GraphicalRepresentation.h>
#include <GUI/QtWidget/EspinaRenderView.h>
#include <Core/Outputs/MeshType.h>

// VTK
#include <vtkSmartPointer.h>


class vtkPolyDataMapper;
class vtkActor;

namespace EspINA
{
  class TransparencySelectionHighlighter;
  class VolumeView;
  
  class MeshRepresentation
  : public SegmentationGraphicalRepresentation
  {
    Q_OBJECT
    public:
      explicit MeshRepresentation(MeshTypeSPtr data,
                                  EspinaRenderView *view);
      virtual ~MeshRepresentation();

      virtual void setColor(const QColor &color);

      virtual void setHighlighted(bool highlighted);

      virtual void setVisible(bool visible);

      virtual bool isInside(Nm point[3]);

      virtual RenderableView canRenderOnView() const
      { return GraphicalRepresentation::VOLUME_VIEW; }

      virtual GraphicalRepresentationSPtr clone(SliceView *view)
      { return GraphicalRepresentationSPtr(); }

      virtual GraphicalRepresentationSPtr clone(VolumeView *view);

      virtual bool hasActor(vtkProp *actor) const;

      virtual void updateRepresentation();

    private slots:
      void updatePipelineConnections();

    private:
      void initializePipeline(VolumeView *view);

    private:
      MeshTypeSPtr m_data;

      vtkSmartPointer<vtkPolyDataMapper> m_mapper;
      vtkSmartPointer<vtkActor>          m_actor;

      static TransparencySelectionHighlighter *s_highlighter;
    };


  typedef boost::shared_ptr<MeshRepresentation> MeshRepresentationSPtr;
  typedef QList<MeshRepresentationSPtr> MeshRepresentationList;

} /* namespace EspINA */
#endif /* MESHREPRESENTATION_H_ */
