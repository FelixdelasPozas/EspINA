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
#include "GUI/Representations/VolumeGPURepresentation.h"
#include "GraphicalRepresentationEmptySettings.h"
#include "GUI/QtWidget/VolumeView.h"
#include <Core/ColorEngines/TransparencySelectionHighlighter.h>

// VTK
#include <vtkGPUVolumeRayCastMapper.h>
#include <vtkMath.h>
#include <vtkVolumeRayCastCompositeFunction.h>
#include <vtkColorTransferFunction.h>
#include <vtkPiecewiseFunction.h>
#include <vtkVolumeProperty.h>
#include <vtkVolume.h>
#include <vtkRenderWindow.h>

using namespace EspINA;

TransparencySelectionHighlighter *VolumeGPURaycastRepresentation::s_highlighter = new TransparencySelectionHighlighter();

//-----------------------------------------------------------------------------
VolumeGPURaycastRepresentation::VolumeGPURaycastRepresentation(SegmentationVolumeSPtr data, EspinaRenderView *view)
: SegmentationGraphicalRepresentation(view)
, m_data(data)
{
  setLabel(tr("Volume GPU"));
}

//-----------------------------------------------------------------------------
VolumeGPURaycastRepresentation::~VolumeGPURaycastRepresentation()
{
}

//-----------------------------------------------------------------------------
GraphicalRepresentationSettings *VolumeGPURaycastRepresentation::settingsWidget()
{
  return new GraphicalRepresentationEmptySettings();
}

//-----------------------------------------------------------------------------
void VolumeGPURaycastRepresentation::setColor(const QColor &color)
{
  SegmentationGraphicalRepresentation::setColor(color);

  if (m_actor != NULL)
  {
    LUTPtr colors = s_highlighter->lut(m_color, m_highlight);

    double rgba[4], rgb[3], hsv[3];
    colors->GetTableValue(1, rgba);
    memcpy(rgb, rgba, 3*sizeof(double));
    vtkMath::RGBToHSV(rgb, hsv);
    m_colorFunction->AddHSVPoint(255, hsv[0], hsv[1], hsv[2]);
  }
}

//-----------------------------------------------------------------------------
void VolumeGPURaycastRepresentation::setHighlighted(bool highlight)
{
  GraphicalRepresentation::setHighlighted(highlight);
  setColor(m_color);
}

//-----------------------------------------------------------------------------
bool VolumeGPURaycastRepresentation::isInside(Nm *point)
{
  // FIXME: unused now, buy maybe useful in the future
  return false;
}

//-----------------------------------------------------------------------------
bool VolumeGPURaycastRepresentation::hasActor(vtkProp *actor) const
{
  if (m_actor == NULL)
    return false;

  return m_actor.GetPointer() == actor;
}

//-----------------------------------------------------------------------------
void VolumeGPURaycastRepresentation::updateRepresentation()
{
  if (m_actor != NULL)
  {
    m_colorFunction->Modified();
    m_actor->Modified();
    m_mapper->UpdateWholeExtent();
    m_actor->Update();
  }
}

//-----------------------------------------------------------------------------
void VolumeGPURaycastRepresentation::updatePipelineConnections()
{
  if ((m_actor != NULL) && (m_mapper->GetInputConnection(0,0) != m_data->toVTK()))
  {
    m_mapper->SetInputConnection(m_data->toVTK());
    m_mapper->Update();
  }
}

//-----------------------------------------------------------------------------
void VolumeGPURaycastRepresentation::initializePipeline()
{
  connect(m_data.get(), SIGNAL(representationChanged()),
          this, SLOT(updatePipelineConnections()));

  itkVolumeType::RegionType region = m_data->toITK()->GetLargestPossibleRegion();
  vtkIdType numPixels = region.GetSize()[0] * region.GetSize()[1] * region.GetSize()[2] + 1024;

  vtkSmartPointer<vtkVolumeRayCastCompositeFunction> composite = vtkSmartPointer<vtkVolumeRayCastCompositeFunction>::New();
  m_mapper = vtkSmartPointer<vtkGPUVolumeRayCastMapper>::New();
  m_mapper->ReleaseDataFlagOn();
  m_mapper->GlobalWarningDisplayOff();
  m_mapper->AutoAdjustSampleDistancesOn();
  m_mapper->SetScalarModeToUsePointData();
  m_mapper->SetBlendModeToComposite();
  m_mapper->SetMaxMemoryFraction(1);
  m_mapper->SetMaxMemoryInBytes(numPixels);
  m_mapper->SetInputConnection(m_data->toVTK());
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
  piecewise->Update();

  vtkSmartPointer<vtkVolumeProperty> property = vtkSmartPointer<vtkVolumeProperty>::New();
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
}

//-----------------------------------------------------------------------------
QList<vtkProp*> VolumeGPURaycastRepresentation::getActors()
{
  QList<vtkProp*> list;

  if (m_actor == NULL)
    initializePipeline();

  list << m_actor.GetPointer();

  return list;
}

//-----------------------------------------------------------------------------
GraphicalRepresentationSPtr VolumeGPURaycastRepresentation::cloneImplementation(VolumeView *view)
{
  VolumeGPURaycastRepresentation *representation = new VolumeGPURaycastRepresentation(m_data, view);
  representation->setView(view);

  return GraphicalRepresentationSPtr(representation);
}

//-----------------------------------------------------------------------------
void VolumeGPURaycastRepresentation::updateVisibility(bool visible)
{
  if (m_actor != NULL)
    m_actor->SetVisibility(visible);
}
