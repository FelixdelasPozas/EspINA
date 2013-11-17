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

// EspINA
#include "VolumetricRepresentation.h"
#include "RepresentationEmptySettings.h"
#include "GUI/View/RenderView.h"
#include "GUI/View/View3D.h"
#include "GUI/ColorEngines/ColorEngine.h"
#include "GUI/ColorEngines/TransparencySelectionHighlighter.h"

// VTK
#include <vtkVolumeRayCastMapper.h>
#include <vtkVolumeRayCastCompositeFunction.h>
#include <vtkColorTransferFunction.h>
#include <vtkVolumeProperty.h>
#include <vtkPiecewiseFunction.h>
#include <vtkVolume.h>
#include <vtkMath.h>

using namespace EspINA;

template<class T> TransparencySelectionHighlighter *VolumetricRepresentation<T>::s_highlighter = new TransparencySelectionHighlighter();

//-----------------------------------------------------------------------------
template<class T>
VolumetricRepresentation<T>::VolumetricRepresentation(VolumetricDataSPtr<T> data, RenderView *view)
: Representation(view)
, m_data(data)
{
  setLabel(tr("Volume Raycast"));
}

//-----------------------------------------------------------------------------
template<class T>
VolumetricRepresentation<T>::~VolumetricRepresentation()
{
}

//-----------------------------------------------------------------------------
template<class T>
RepresentationSettings *VolumetricRepresentation<T>::settingsWidget()
{
  return new RepresentationEmptySettings();
}

//-----------------------------------------------------------------------------
template<class T>
void VolumetricRepresentation<T>::setColor(const QColor &color)
{
  Representation::setColor(color);

  if (m_actor != nullptr)
  {
    LUTSPtr colors = s_highlighter->lut(m_color, m_highlight);
    double rgba[4], rgb[3], hsv[3];
    colors->GetTableValue(1, rgba);
    memcpy(rgb, rgba, 3*sizeof(double));
    vtkMath::RGBToHSV(rgb, hsv);
    m_colorFunction->AddHSVPoint(255, hsv[0], hsv[1], hsv[2]);
  }
}

//-----------------------------------------------------------------------------
template<class T>
void VolumetricRepresentation<T>::setHighlighted(bool highlight)
{
  Representation::setHighlighted(highlight);
  setColor(m_color);
}

//-----------------------------------------------------------------------------
template<class T>
bool VolumetricRepresentation<T>::isInside(const NmVector3 &point) const
{
  // FIXME: unused now, buy maybe useful in the future
  return false;
}

//-----------------------------------------------------------------------------
template<class T>
bool VolumetricRepresentation<T>::hasActor(vtkProp *actor) const
{
  if (m_actor == nullptr)
    return false;

  return m_actor.GetPointer() == actor;
}

//-----------------------------------------------------------------------------
template<class T>
void VolumetricRepresentation<T>::updateRepresentation()
{
  if (m_actor != nullptr)
  {
    m_exporter->SetInput(m_data->itkImage());
    m_exporter->Update();
    m_mapper->UpdateWholeExtent();
    m_colorFunction->Modified();
    m_actor->Modified();
    m_actor->Update();
  }
}

//-----------------------------------------------------------------------------
template<class T>
void VolumetricRepresentation<T>::initializePipeline()
{
  itkVolumeType::Pointer volume = m_data->itkImage();
  m_exporter = ExporterType::New();
  m_exporter->ReleaseDataFlagOn();
  m_exporter->SetNumberOfThreads(1);
  m_exporter->SetInput(volume);
  m_exporter->Update();

  vtkSmartPointer<vtkVolumeRayCastCompositeFunction> composite = vtkSmartPointer<vtkVolumeRayCastCompositeFunction>::New();
  m_mapper = vtkSmartPointer<vtkVolumeRayCastMapper>::New();
  m_mapper->ReleaseDataFlagOn();
  m_mapper->SetBlendModeToComposite();
  m_mapper->SetVolumeRayCastFunction(composite);
  m_mapper->IntermixIntersectingGeometryOff();
  m_mapper->SetInputData(m_exporter->GetOutput());
  m_mapper->Update();

  // actor should be allocated first of the next call to setColor would do nothing
  m_actor = vtkSmartPointer<vtkVolume>::New();

  m_colorFunction = vtkSmartPointer<vtkColorTransferFunction>::New();
  m_colorFunction->AllowDuplicateScalarsOff();
  setColor(m_color);
  m_colorFunction->Modified();

  vtkSmartPointer<vtkPiecewiseFunction> piecewise = vtkSmartPointer<vtkPiecewiseFunction>::New();
  piecewise->AddPoint(0, 0.0);
  piecewise->AddPoint(255, 1.0);
  Q_ASSERT(false);//TODO 2013-10-08 piecewise->Update();

  vtkSmartPointer<vtkVolumeProperty> property = vtkSmartPointer<vtkVolumeProperty>::New();
  property->SetColor(m_colorFunction);
  property->SetScalarOpacity(piecewise);
  property->DisableGradientOpacityOff();
  property->SetSpecular(0.5);
  property->ShadeOn();
  property->SetInterpolationTypeToLinear();
  property->Modified();

  m_actor->SetMapper(m_mapper);
  m_actor->SetProperty(property);
  m_actor->SetVisibility(isVisible());
  m_actor->Update();
}

//-----------------------------------------------------------------------------
template<class T>
QList<vtkProp*> VolumetricRepresentation<T>::getActors()
{
  QList<vtkProp*> list;

  if (m_actor == nullptr)
    initializePipeline();

  list << m_actor.GetPointer();

  return list;
}

//-----------------------------------------------------------------------------
template<class T>
RepresentationSPtr VolumetricRepresentation<T>::cloneImplementation(View3D *view)
{
  VolumetricRepresentation *representation = new VolumetricRepresentation(m_data, view);
  representation->setView(view);

  return RepresentationSPtr(representation);
}

//-----------------------------------------------------------------------------
template<class T>
void VolumetricRepresentation<T>::updateVisibility(bool visible)
{
  if (m_actor != nullptr)
    m_actor->SetVisibility(visible);
}
