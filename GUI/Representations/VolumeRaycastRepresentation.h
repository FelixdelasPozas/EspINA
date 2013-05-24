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

#ifndef VOLUMERAYCASTREPRESENTATION_H_
#define VOLUMERAYCASTREPRESENTATION_H_

// EspINA
#include <Core/Outputs/VolumeRepresentation.h>
#include "GUI/Representations/GraphicalRepresentation.h"

// VTK
#include <vtkSmartPointer.h>

class vtkVolumeRayCastMapper;
class vtkColorTransferFunction;
class vtkVolume;

namespace EspINA
{
  class TransparencySelectionHighlighter;
  class VolumeView;
  
  class VolumeRaycastRepresentation
  : public SegmentationGraphicalRepresentation
  {
    Q_OBJECT
    public:
      explicit VolumeRaycastRepresentation(SegmentationVolumeSPtr data,
                                           EspinaRenderView *view);
      virtual ~VolumeRaycastRepresentation();

      virtual void setColor(const QColor &color);

      virtual void setHighlighted(bool highlighted);

      virtual void setVisible(bool visible);

      virtual bool isInside(Nm point[3]);

      virtual RenderableView canRenderOnView() const
      { return GraphicalRepresentation::RENDERABLEVIEW_VOLUME; }

      virtual GraphicalRepresentationSPtr clone(SliceView *view)
      { return GraphicalRepresentationSPtr(); }

      virtual GraphicalRepresentationSPtr clone(VolumeView *view);

      virtual bool hasActor(vtkProp *actor) const;

      virtual void updateRepresentation();

      virtual QList<vtkProp*> getActors();

    private slots:
      void updatePipelineConnections();

    private:
      void initializePipeline(VolumeView *view);

    private:
      SegmentationVolumeSPtr m_data;

      vtkSmartPointer<vtkVolumeRayCastMapper>   m_mapper;
      vtkSmartPointer<vtkColorTransferFunction> m_colorFunction;
      vtkSmartPointer<vtkVolume>                m_actor;

      static TransparencySelectionHighlighter *s_highlighter;
    };


  typedef boost::shared_ptr<VolumeRaycastRepresentation> VolumeRaycastRepresentationSPtr;
  typedef QList<VolumeRaycastRepresentationSPtr> VolumeRaycastRepresentationSList;

} /* namespace EspINA */
#endif /* VOLUMERAYCASTREPRESENTATION_H_ */
