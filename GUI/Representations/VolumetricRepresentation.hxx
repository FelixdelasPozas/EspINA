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

#include "GUI/EspinaGUI_Export.h"

// ESPINA
#include <Core/Analysis/Data/VolumetricData.hxx>
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

namespace ESPINA
{
  class TransparencySelectionHighlighter;

  template<class T>
  class EspinaGUI_EXPORT VolumetricRepresentation
  : public Representation
  {
    public:
      static const Representation::Type TYPE;

    public:
      /* \brief VolumetricRepresentation class constructor.
       * \param[in] data, volumetric data smart pointer of the data to represent.
       * \param[in] view, renderview raw pointer the representation will be shown.
       *
       */
      explicit VolumetricRepresentation(VolumetricDataSPtr<T> data,
                                        RenderView *view);

      /* \brief VolumetricRepresentation class virtual destructor.
       *
       */
      virtual ~VolumetricRepresentation();

      /* \brief Implements Representation::settingsWidget().
       *
       */
      virtual RepresentationSettings *settingsWidget();

      /* \brief Overrides Representation::setColor().
       *
       */
      virtual void setColor(const QColor &color) override;

      /* \brief Overrides Representation::setHighlighted().
       *
       */
      virtual void setHighlighted(bool highlighted) override;

      /* \brief Implements Representation::isInside() const.
       *
       */
      virtual bool isInside(const NmVector3 &point) const;

      /* \brief Implements Representation::canRenderOnView() const.
       *
       */
      virtual RenderableView canRenderOnView() const
      { return Representation::RENDERABLEVIEW_VOLUME; }

      /* \brief Implements Representation::hasActor() const.
       *
       */
      virtual bool hasActor(vtkProp *actor) const;

      /* \brief Implements Representation::updateRepresentation().
       *
       */
      virtual void updateRepresentation();

      /* \brief Implements Representation::getActors().
       *
       */
      virtual QList<vtkProp*> getActors();

      /* \brief Implements Representation::crosshairDependent().
       *
       */
      virtual bool crosshairDependent() const;

      /* \brief Implements Representation::needUpdate().
       *
       */
      virtual bool needUpdate() const
      { return m_lastUpdatedTime != m_data->lastModified(); }

    protected:
      /* \brief Implements Representation::cloneImplementation(View2D*).
       *
       */
      virtual RepresentationSPtr cloneImplementation(View2D *view)
      { return RepresentationSPtr(); }

      /* \brief Implements Representation::cloneImplementation(View3D*).
       *
       */
      virtual RepresentationSPtr cloneImplementation(View3D *view);

      /* \brief Implements Representation::updateVisibility().
       *
       */
      virtual void updateVisibility(bool visible);

    private:
      /* \brief Helper method to set the view.
       * \param[in] view, View3D raw pointer.
       *
       */
      void setView(View3D *view)
      { m_view = view; }

      /* \brief Helper method to initialize the vtk pipeline.
       *
       */
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
} /* namespace ESPINA */

#include "VolumetricRepresentation.cxx"

#endif /* ESPINA_VOLUMETRIC_REPRESENTATION_H_ */
