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

#ifndef ESPINA_CROSSHAIR_REPRESENTATION_H
#define ESPINA_CROSSHAIR_REPRESENTATION_H

#include "EspinaGUI_Export.h"

// EspINA
#include "GUI/Representations/Representation.h"
#include <Core/Analysis/Data/VolumetricData.h>
#include <GUI/View/VolumeView.h>

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
  class View2D;
  class View3D;
  class RenderView;
  class CrosshairRenderer;

  using ChannelVolumeSPtr = std::shared_ptr<VolumetricData<itkVolumeType>>;

  class EspinaGUI_EXPORT CrosshairRepresentation
  :public Representation
  {
    Q_OBJECT
    public:
      explicit CrosshairRepresentation(ChannelVolumeSPtr data,
                                       View3D       *view);
      virtual ~CrosshairRepresentation() {};

      virtual void setBrightness(double value);

      virtual void setContrast(double value);

      virtual RepresentationSettings *settingsWidget();

      virtual void setColor(const QColor &color);

      virtual void setOpacity(double value);

      virtual bool isInside(Nm point[3]);

      virtual RenderableView canRenderOnView() const
      { return Representation::RENDERABLEVIEW_VOLUME; }

      virtual bool hasActor(vtkProp *actor) const;

      virtual void updateRepresentation();

      virtual QList<vtkProp*> getActors();

      void setCrosshairColors(double axialColor[3], double coronalColor[3], double sagittalColor[3]);
      void setCrosshair(Nm point[3]);
      void setPlanePosition(PlaneType plane, Nm dist);

      bool tiling()              { return m_tiling; }
      void setTiling(bool value) { m_tiling = value; }

  protected:
      virtual RepresentationSPtr cloneImplementation(View2D *view)
      { return RepresentationSPtr(); }

      virtual RepresentationSPtr cloneImplementation(View3D *view);

    virtual void updateVisibility(bool visible);

    private slots:
      void updatePipelineConnections();

    private:
      void setView(View3D *view) { m_view = view; };
      void initializePipeline();

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

} // namespace EspINA

#endif // ESPINA_CROSSHAIR_REPRESENTATION_H
