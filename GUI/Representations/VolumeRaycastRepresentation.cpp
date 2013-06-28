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
#include "VolumeRaycastRepresentation.h"
#include "GraphicalRepresentationEmptySettings.h"
#include "GUI/QtWidget/EspinaRenderView.h"
#include "GUI/QtWidget/VolumeView.h"
#include "Core/ColorEngines/IColorEngine.h"
#include "Core/ColorEngines/TransparencySelectionHighlighter.h"

// VTK
#include <vtkVolumeRayCastMapper.h>
#include <vtkVolumeRayCastCompositeFunction.h>
#include <vtkColorTransferFunction.h>
#include <vtkVolumeProperty.h>
#include <vtkPiecewiseFunction.h>
#include <vtkVolume.h>
#include <vtkMath.h>

using namespace EspINA;

TransparencySelectionHighlighter *VolumeRaycastRepresentation::s_highlighter = new TransparencySelectionHighlighter();

//-----------------------------------------------------------------------------
VolumeRaycastRepresentation::VolumeRaycastRepresentation(SegmentationVolumeSPtr data, EspinaRenderView *view)
: SegmentationGraphicalRepresentation(view)
, m_data(data)
{
  setLabel(tr("Volume Raycast"));
}

//-----------------------------------------------------------------------------
VolumeRaycastRepresentation::~VolumeRaycastRepresentation()
{
}

//-----------------------------------------------------------------------------
GraphicalRepresentationSettings *VolumeRaycastRepresentation::settingsWidget()
{
  return new GraphicalRepresentationEmptySettings();
}

//-----------------------------------------------------------------------------
void VolumeRaycastRepresentation::setColor(const QColor &color)
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
void VolumeRaycastRepresentation::setHighlighted(bool highlight)
{
  GraphicalRepresentation::setHighlighted(highlight);
  setColor(m_color);
}

//-----------------------------------------------------------------------------
bool VolumeRaycastRepresentation::isInside(Nm *point)
{
  // FIXME: unused now, buy maybe useful in the future
  return false;
}

//-----------------------------------------------------------------------------
bool VolumeRaycastRepresentation::hasActor(vtkProp *actor) const
{
  if (m_actor == NULL)
    return false;

  return m_actor.GetPointer() == actor;
}

//-----------------------------------------------------------------------------
void VolumeRaycastRepresentation::updateRepresentation()
{
  if (m_actor != NULL)
  {
    m_mapper->Update();
    m_colorFunction->Modified();
    m_actor->Modified();
    m_actor->Update();
  }
}

//-----------------------------------------------------------------------------
void VolumeRaycastRepresentation::updatePipelineConnections()
{
  if ((m_actor != NULL) && (m_mapper->GetInputConnection(0,0) != m_data->toVTK()))
  {
    m_mapper->SetInputConnection(m_data->toVTK());
    m_mapper->Update();
  }
}

//-----------------------------------------------------------------------------
void VolumeRaycastRepresentation::initializePipeline()
{
  connect(m_data.get(), SIGNAL(representationChanged()),
          this, SLOT(updatePipelineConnections()));

  vtkSmartPointer<vtkVolumeRayCastCompositeFunction> composite = vtkSmartPointer<vtkVolumeRayCastCompositeFunction>::New();
  m_mapper = vtkSmartPointer<vtkVolumeRayCastMapper>::New();
  m_mapper->ReleaseDataFlagOn();
  m_mapper->SetBlendModeToComposite();
  m_mapper->SetVolumeRayCastFunction(composite);
  m_mapper->IntermixIntersectingGeometryOff();
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
  property->Modified();

  m_actor->SetMapper(m_mapper);
  m_actor->SetProperty(property);
  m_actor->SetVisibility(isVisible());
  m_actor->Update();
}

//-----------------------------------------------------------------------------
QList<vtkProp*> VolumeRaycastRepresentation::getActors()
{
  QList<vtkProp*> list;

  if (m_actor == NULL)
    initializePipeline();

  list << m_actor.GetPointer();

  return list;
}

//-----------------------------------------------------------------------------
GraphicalRepresentationSPtr VolumeRaycastRepresentation::cloneImplementation(VolumeView *view)
{
  VolumeRaycastRepresentation *representation = new VolumeRaycastRepresentation(m_data, view);
  representation->setView(view);

  return GraphicalRepresentationSPtr(representation);
}

//-----------------------------------------------------------------------------
void VolumeRaycastRepresentation::updateVisibility(bool visible)
{
  if (m_actor != NULL)
    m_actor->SetVisibility(visible);
}
