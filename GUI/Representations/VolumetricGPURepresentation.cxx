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

namespace ESPINA
{
  template<class T>
  const Representation::Type VolumetricGPURepresentation<T>::TYPE = "Volumetric GPU";

  template<class T>
  TransparencySelectionHighlighter *VolumetricGPURepresentation<T>::s_highlighter = new TransparencySelectionHighlighter();

  //-----------------------------------------------------------------------------
  template<class T>
  VolumetricGPURepresentation<T>::VolumetricGPURepresentation(VolumetricDataSPtr<T> data, RenderView *view)
  : Representation{view}
  , m_data        {data}
  {
    setType(TYPE);
  }

  //-----------------------------------------------------------------------------
  template<class T>
  VolumetricGPURepresentation<T>::~VolumetricGPURepresentation()
  {
  }

  //-----------------------------------------------------------------------------
  template<class T>
  RepresentationSettings *VolumetricGPURepresentation<T>::settingsWidget()
  {
    return new RepresentationEmptySettings();
  }

  //-----------------------------------------------------------------------------
  template<class T>
  void VolumetricGPURepresentation<T>::setColor(const QColor &color)
  {
    Representation::setColor(color);

    if (m_actor != nullptr)
    {
      auto colors = s_highlighter->lut(m_color, m_highlight);
      double rgba[4], rgb[3], hsv[3];
      colors->GetTableValue(1, rgba);
      memcpy(rgb, rgba, 3 * sizeof(double));
      vtkMath::RGBToHSV(rgb, hsv);
      m_colorFunction->AddHSVPoint(255, hsv[0], hsv[1], hsv[2]);
    }
  }

  //-----------------------------------------------------------------------------
  template<class T>
  void VolumetricGPURepresentation<T>::setHighlighted(bool highlight)
  {
    Representation::setHighlighted(highlight);
    setColor(m_color);
  }

  //-----------------------------------------------------------------------------
  template<class T>
  bool VolumetricGPURepresentation<T>::isInside(const NmVector3 &point) const
  {
    // TODO: not useful now, maybe in the future
    return false;
  }

  //-----------------------------------------------------------------------------
  template<class T>
  bool VolumetricGPURepresentation<T>::hasActor(vtkProp *actor) const
  {
    if (m_actor == nullptr)
      return false;

    return m_actor.GetPointer() == actor;
  }

  //-----------------------------------------------------------------------------
  template<class T>
  void VolumetricGPURepresentation<T>::updateRepresentation()
  {
    if (isVisible() && (m_actor != nullptr) && needUpdate())
    {
      auto volume = vtkImage(m_data, m_data->bounds());
      m_mapper->SetInputData(volume);
      m_mapper->Update();
      m_colorFunction->Modified();
      m_actor->Update();

      m_lastUpdatedTime = m_data->lastModified();
    }
  }

  //-----------------------------------------------------------------------------
  template<class T>
  void VolumetricGPURepresentation<T>::initializePipeline()
  {
    auto volume = vtkImage(m_data, m_data->bounds());

    auto composite = vtkSmartPointer<vtkVolumeRayCastCompositeFunction>::New();
    m_mapper = vtkSmartPointer<vtkGPUVolumeRayCastMapper>::New();
    m_mapper->ReleaseDataFlagOn();
    m_mapper->GlobalWarningDisplayOff();
    m_mapper->AutoAdjustSampleDistancesOn();
    m_mapper->SetScalarModeToUsePointData();
    m_mapper->SetBlendModeToComposite();
    m_mapper->SetMaxMemoryFraction(1);
    m_mapper->SetInputData(volume);
    m_mapper->Update();

    // actor should be allocated first of the next call to setColor would do nothing
    m_actor = vtkSmartPointer<vtkVolume>::New();

    m_colorFunction = vtkSmartPointer<vtkColorTransferFunction>::New();
    m_colorFunction->AllowDuplicateScalarsOff();
    setColor(m_color);
    m_colorFunction->Modified();

    auto piecewise = vtkSmartPointer<vtkPiecewiseFunction>::New();
    piecewise->AddPoint(0, 0.0);
    piecewise->AddPoint(SEG_VOXEL_VALUE, 1.0);
    piecewise->Modified();

    auto property = vtkSmartPointer<vtkVolumeProperty>::New();
    property->SetColor(m_colorFunction);
    property->SetScalarOpacity(piecewise);
    property->DisableGradientOpacityOff();
    property->SetSpecular(0.5);
    property->ShadeOn();
    property->SetInterpolationTypeToLinear();
    property->IndependentComponentsOn();
    property->Modified();

    m_actor->UseBoundsOn();
    m_actor->PickableOn();
    m_actor->SetMapper(m_mapper);
    m_actor->SetProperty(property);
    m_actor->SetVisibility(isVisible());
    m_actor->Update();

    m_lastUpdatedTime = m_data->lastModified();
  }

  //-----------------------------------------------------------------------------
  template<class T>
  QList<vtkProp*> VolumetricGPURepresentation<T>::getActors()
  {
    QList<vtkProp*> list;

    if (m_actor == nullptr)
      initializePipeline();

    list << m_actor.GetPointer();

    return list;
  }

  //-----------------------------------------------------------------------------
  template<class T>
  RepresentationSPtr VolumetricGPURepresentation<T>::cloneImplementation(View3D *view)
  {
    VolumetricGPURepresentation *representation = new VolumetricGPURepresentation(m_data, view);
    representation->setView(view);

    return RepresentationSPtr(representation);
  }

  //-----------------------------------------------------------------------------
  template<class T>
  void VolumetricGPURepresentation<T>::updateVisibility(bool visible)
  {
    if(visible && m_actor != nullptr && needUpdate())
      updateRepresentation();

    if (m_actor != nullptr)
      m_actor->SetVisibility(visible);
  }

  //-----------------------------------------------------------------------------
  template<class T>
  bool VolumetricGPURepresentation<T>::crosshairDependent() const
  {
    return false;
  }
}
