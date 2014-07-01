/*
 
 Copyright (C) 2014 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

 This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
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

#ifndef ESPINA_VOLUMETRIC_REPRESENTATION_H_
#define ESPINA_VOLUMETRIC_REPRESENTATION_H_

#include "EspinaGUI_Export.h"

// EspINA
#include "VolumetricRepresentation.h"
#include <Core/Analysis/Data/VolumetricData.h>
#include "RepresentationEmptySettings.h"
#include "GUI/Representations/Representation.h"
#include "GUI/View/RenderView.h"
#include "GUI/View/View3D.h"
#include "GUI/ColorEngines/ColorEngine.h"
#include "GUI/ColorEngines/TransparencySelectionHighlighter.h"

// VTK
#include <vtkSmartPointer.h>
#include <vtkVolumeRayCastMapper.h>
#include <vtkVolumeRayCastCompositeFunction.h>
#include <vtkColorTransferFunction.h>
#include <vtkVolumeProperty.h>
#include <vtkPiecewiseFunction.h>
#include <vtkVolume.h>
#include <vtkMath.h>

class vtkVolumeRayCastMapper;
class vtkColorTransferFunction;
class vtkVolume;

namespace EspINA
{
  class TransparencySelectionHighlighter;
  
  template<class T>
  class EspinaGUI_EXPORT VolumetricRepresentation
  : public Representation
  {
    public:
      static const Representation::Type TYPE;

    public:
      explicit VolumetricRepresentation(VolumetricDataSPtr<T> data,
                                        RenderView *view);
      virtual ~VolumetricRepresentation();

      virtual RepresentationSettings *settingsWidget();

      virtual void setColor(const QColor &color);

      virtual void setHighlighted(bool highlighted);

      virtual bool isInside(const NmVector3 &point) const;

      virtual RenderableView canRenderOnView() const
      { return Representation::RENDERABLEVIEW_VOLUME; }

      virtual bool hasActor(vtkProp *actor) const;

      virtual void updateRepresentation();

      virtual QList<vtkProp*> getActors();

      virtual bool crosshairDependent() const;

      virtual bool needUpdate()
      { return m_lastUpdatedTime != m_data->lastModified(); }

    protected:
      virtual RepresentationSPtr cloneImplementation(View2D *view)
      { return RepresentationSPtr(); }

      virtual RepresentationSPtr cloneImplementation(View3D *view);

      virtual void updateVisibility(bool visible);

    private:
      void setView(View3D *view) { m_view = view; }
      void initializePipeline();

    private:
      VolumetricDataSPtr<T>                     m_data;
      vtkSmartPointer<vtkVolumeRayCastMapper>   m_mapper;
      vtkSmartPointer<vtkColorTransferFunction> m_colorFunction;
      vtkSmartPointer<vtkVolume>                m_actor;

      static TransparencySelectionHighlighter *s_highlighter;
  };

  template<class T> using VolumetricRepresentationPtr  = VolumetricRepresentation<T> *;
  template<class T> using VolumetricRepresentationSPtr = std::shared_ptr<VolumetricRepresentation<T>>;
  template<class T> using VolumetricRepresentationSList = QList<VolumetricRepresentationSPtr<T>>;

  template class VolumetricRepresentation<itkVolumeType>;
} /* namespace EspINA */

#include "VolumetricRepresentation.txx"

#endif /* ESPINA_VOLUMETRIC_REPRESENTATION_H_ */
