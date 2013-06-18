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

#ifndef VOLUMEGPUREPRESENTATION_H_
#define VOLUMEGPUREPRESENTATION_H_

#include "EspinaGUI_Export.h"

// EspINA
#include "GraphicalRepresentation.h"
#include "GUI/QtWidget/EspinaRenderView.h"
#include <Core/OutputRepresentations/VolumeRepresentation.h>
#include <GUI/QtWidget/VolumeView.h>

// VTK
#include <vtkSmartPointer.h>

class vtkGPUVolumeRayCastMapper;
class vtkColorTransferFunction;
class vtkVolume;

namespace EspINA
{
  class TransparencySelectionHighlighter;
  class VolumeView;
  
  class EspinaGUI_EXPORT VolumeGPURaycastRepresentation
  : public SegmentationGraphicalRepresentation
  {
    Q_OBJECT
    public:
      explicit VolumeGPURaycastRepresentation(SegmentationVolumeSPtr data,
                                              EspinaRenderView *view);
      virtual ~VolumeGPURaycastRepresentation();

      virtual GraphicalRepresentationSettings *settingsWidget();

      virtual void setColor(const QColor &color);

      virtual void setHighlighted(bool highlighted);

      virtual bool isInside(Nm point[3]);

      virtual RenderableView canRenderOnView() const
      { return GraphicalRepresentation::RENDERABLEVIEW_VOLUME; }

      virtual bool hasActor(vtkProp *actor) const;

      virtual void updateRepresentation();

      virtual QList<vtkProp*> getActors();

  protected:
      virtual GraphicalRepresentationSPtr cloneImplementation(SliceView *view)
      { return GraphicalRepresentationSPtr(); }

      virtual GraphicalRepresentationSPtr cloneImplementation(VolumeView *view);

    virtual void updateVisibility(bool visible);

    private slots:
      void updatePipelineConnections();

    private:
      void setView(VolumeView *view) { m_view = view; };
      void initializePipeline();

    private:
      SegmentationVolumeSPtr m_data;

      vtkSmartPointer<vtkGPUVolumeRayCastMapper> m_mapper;
      vtkSmartPointer<vtkColorTransferFunction>  m_colorFunction;
      vtkSmartPointer<vtkVolume>                 m_actor;

      static TransparencySelectionHighlighter *s_highlighter;
    };

  typedef boost::shared_ptr<VolumeGPURaycastRepresentation> VolumeGPURepresentationSPtr;
  typedef QList<VolumeGPURepresentationSPtr> VolumeGPURepresentationSList;

} /* namespace EspINA */
#endif /* VOLUMEGPUREPRESENTATION_H_ */
