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

#ifndef IMESHREPRESENTATION_H_
#define IMESHREPRESENTATION_H_

// EspINA
#include "GraphicalRepresentation.h"
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
  
  class IMeshRepresentation
  : public SegmentationGraphicalRepresentation
  {
    Q_OBJECT
    public:
      explicit IMeshRepresentation(MeshTypeSPtr data,
                                   EspinaRenderView *view);
      virtual ~IMeshRepresentation();

      virtual void setColor(const QColor &color);

      virtual void setHighlighted(bool highlighted);

      virtual void setVisible(bool visible);

      virtual bool isInside(Nm point[3]);

      virtual RenderableView canRenderOnView() const
      { return GraphicalRepresentation::VOLUME_VIEW; }

      virtual GraphicalRepresentationSPtr clone(SliceView *view)
      { return GraphicalRepresentationSPtr(); }

      virtual GraphicalRepresentationSPtr clone(VolumeView *view) = 0;

      virtual bool hasActor(vtkProp *actor) const;

      virtual void updateRepresentation() = 0;

    private slots:
      virtual void updatePipelineConnections() = 0;

    private:
      virtual void initializePipeline(VolumeView *view) = 0;

    protected:
      MeshTypeSPtr m_data;
      vtkSmartPointer<vtkPolyDataMapper> m_mapper;
      vtkSmartPointer<vtkActor>          m_actor;

      static TransparencySelectionHighlighter *s_highlighter;
  };

  // one shouldn't address objects of this class directly but use the subclasses,
  // however this is here for convenience.
  typedef boost::shared_ptr<IMeshRepresentation> IMeshRepresentationSPtr;
  typedef QList<IMeshRepresentationSPtr> IMeshRepresentationSList;

} /* namespace EspINA */
#endif /* IMESHREPRESENTATION_H_ */
