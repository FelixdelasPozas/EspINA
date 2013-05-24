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

#ifndef CROSSHAIRREPRESENTATION_H_
#define CROSSHAIRREPRESENTATION_H_

// EspINA
#include "GUI/Representations/GraphicalRepresentation.h"
#include <Core/EspinaTypes.h>
#include <Core/Outputs/VolumeRepresentation.h>

// VTK
#include <vtkSmartPointer.h>

class QColor;
class vtkActor;
class vtkImageActor;
class vtkImageReslice;
class vtkPolyData;
class vtkMatrix4x4;
class vtkLookupTable;
class vtkImageShiftScale;

namespace EspINA
{
  class SliceView;
  class VolumeView;
  class EspinaRenderView;
  class CrosshairRenderer;
  
  class CrosshairRepresentation
  :public ChannelGraphicalRepresentation
  {
    Q_OBJECT
    public:
      explicit CrosshairRepresentation(ChannelVolumeSPtr data,
                                       VolumeView       *view);
      virtual ~CrosshairRepresentation() {};

      virtual void setBrightness(double value);

      virtual void setContrast(double value);

      virtual void setColor(const QColor &color);

      virtual void setOpacity(double value);

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

      void setCrosshairColors(double axialColor[3], double coronalColor[3], double sagittalColor[3]);
      void setCrosshair(Nm point[3]);
      void setPlanePosition(PlaneType plane, Nm dist);

      bool tiling()              { return m_tiling; }
      void setTiling(bool value) { m_tiling = value; }

    private slots:
      void updatePipelineConnections();

    private:
      void initializePipeline(EspinaRenderView *view);

    private:
      ChannelVolumeSPtr                   m_data;

      vtkSmartPointer<vtkImageReslice>    m_axialReslice;
      vtkSmartPointer<vtkImageReslice>    m_coronalReslice;
      vtkSmartPointer<vtkImageReslice>    m_sagittalReslice;
      vtkSmartPointer<vtkImageActor>      m_axial;
      vtkSmartPointer<vtkImageActor>      m_coronal;
      vtkSmartPointer<vtkImageActor>      m_sagittal;
      vtkSmartPointer<vtkActor>           m_axialBorder;
      vtkSmartPointer<vtkActor>           m_coronalBorder;
      vtkSmartPointer<vtkActor>           m_sagittalBorder;
      vtkSmartPointer<vtkPolyData>        m_axialSquare;
      vtkSmartPointer<vtkPolyData>        m_coronalSquare;
      vtkSmartPointer<vtkPolyData>        m_sagittalSquare;
      vtkSmartPointer<vtkMatrix4x4>       m_matAxial;
      vtkSmartPointer<vtkMatrix4x4>       m_matCoronal;
      vtkSmartPointer<vtkMatrix4x4>       m_matSagittal;
      vtkSmartPointer<vtkLookupTable>     m_lut;
      vtkSmartPointer<vtkImageShiftScale> m_axialScaler;
      vtkSmartPointer<vtkImageShiftScale> m_coronalScaler;
      vtkSmartPointer<vtkImageShiftScale> m_sagittalScaler;

      double m_bounds[6];
      Nm m_point[3];
      bool m_tiling;

      friend class CrosshairRenderer;
  };

  typedef boost::shared_ptr<CrosshairRepresentation> CrosshairRepresentationSPtr;
  typedef QList<CrosshairRepresentationSPtr> CrosshairRepresentationSList;

} /* namespace EspINA */
#endif /* CROSSHAIRREPRESENTATION_H_ */
