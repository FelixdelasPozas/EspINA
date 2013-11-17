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

#ifndef VOLUMEGPUREPRESENTATION_H_
#define VOLUMEGPUREPRESENTATION_H_

#include "EspinaGUI_Export.h"

// EspINA
#include "Representation.h"
#include "GUI/View/RenderView.h"
#include <Core/Analysis/Data/VolumetricData.h>
#include <GUI/View/View3D.h>

// VTK
#include <vtkSmartPointer.h>

// ITK
#include <itkImageToVTKImageFilter.h>

class vtkGPUVolumeRayCastMapper;
class vtkColorTransferFunction;
class vtkVolume;

namespace EspINA
{
  class TransparencySelectionHighlighter;
  class VolumeView;
  
  template<class T>
  class EspinaGUI_EXPORT VolumetricGPURepresentation
  : public Representation
  {
    public:
      explicit VolumetricGPURepresentation(VolumetricDataSPtr<T> data,
                                           RenderView *view);
      virtual ~VolumetricGPURepresentation();

      virtual RepresentationSettings *settingsWidget();

      virtual void setColor(const QColor &color);

      virtual void setHighlighted(bool highlighted);

      virtual bool isInside(const NmVector3 &point) const;

      virtual RenderableView canRenderOnView() const
      { return Representation::RENDERABLEVIEW_VOLUME; }

      virtual bool hasActor(vtkProp *actor) const;

      virtual void updateRepresentation();

      virtual QList<vtkProp*> getActors();

  protected:
      virtual RepresentationSPtr cloneImplementation(View2D *view)
      { return RepresentationSPtr(); }

      virtual RepresentationSPtr cloneImplementation(View3D *view);

    virtual void updateVisibility(bool visible);

    private:
      void setView(View3D *view) { m_view = view; };
      void initializePipeline();

    private:
      VolumetricDataSPtr<T> m_data;

      using ExporterType = itk::ImageToVTKImageFilter<itkVolumeType>;

      ExporterType::Pointer                      m_exporter;
      vtkSmartPointer<vtkGPUVolumeRayCastMapper> m_mapper;
      vtkSmartPointer<vtkColorTransferFunction>  m_colorFunction;
      vtkSmartPointer<vtkVolume>                 m_actor;

      static TransparencySelectionHighlighter *s_highlighter;
    };

  template<class T> using VolumetricGPURepresentationPtr  = VolumetricGPURepresentation<T> *;
  template<class T> using VolumetricGPURepresentationSPtr = std::shared_ptr<VolumetricGPURepresentation<T>>;
  template<class T> using VolumetricGPURepresentationSList = QList<VolumetricGPURepresentationSPtr<T>>;

} /* namespace EspINA */
#endif /* VOLUMEGPUREPRESENTATION_H_ */
