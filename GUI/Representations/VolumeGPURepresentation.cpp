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

namespace EspINA
{
  TransparencySelectionHighlighter *VolumeGPURaycastRepresentation::s_highlighter = new TransparencySelectionHighlighter();
  
  //-----------------------------------------------------------------------------
  VolumeGPURaycastRepresentation::VolumeGPURaycastRepresentation(SegmentationVolumeSPtr data, EspinaRenderView *view)
  : SegmentationGraphicalRepresentation(view)
  , m_data(data)
  {
  }
  
  //-----------------------------------------------------------------------------
  VolumeGPURaycastRepresentation::~VolumeGPURaycastRepresentation()
  {
    // this is needed or we'll have a memory leak
    if (m_mapper)
      m_mapper->ReleaseGraphicsResources(m_view->renderWindow());
  }

  //-----------------------------------------------------------------------------
  void VolumeGPURaycastRepresentation::setColor(const QColor &color)
  {
    SegmentationGraphicalRepresentation::setColor(color);

    LUTPtr colors = s_highlighter->lut(m_color, m_highlight);

    double rgba[4], rgb[3], hsv[3];
    colors->GetTableValue(1, rgba);
    memcpy(rgb, rgba, 3*sizeof(double));
    vtkMath::RGBToHSV(rgb, hsv);
    m_colorFunction->AddHSVPoint(255, hsv[0], hsv[1], hsv[2]);
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
    return m_actor.GetPointer() == actor;
  }

  //-----------------------------------------------------------------------------
  void VolumeGPURaycastRepresentation::updateRepresentation()
  {
    m_colorFunction->Modified();
    m_actor->Modified();
    m_mapper->Update();
    m_actor->Update();
  }

  //-----------------------------------------------------------------------------
  void VolumeGPURaycastRepresentation::updatePipelineConnections()
  {
    if (m_mapper->GetInputConnection(0,0) != m_data->toVTK())
    {
      m_mapper->SetInputConnection(m_data->toVTK());
      m_mapper->Update();
    }
  }

  //-----------------------------------------------------------------------------
  void VolumeGPURaycastRepresentation::initializePipeline(VolumeView *view)
  {
    connect(m_data.get(), SIGNAL(representationChanged()),
            this, SLOT(updatePipelineConnections()));

    vtkSmartPointer<vtkVolumeRayCastCompositeFunction> composite = vtkSmartPointer<vtkVolumeRayCastCompositeFunction>::New();
    m_mapper = vtkSmartPointer<vtkGPUVolumeRayCastMapper>::New();
    m_mapper->ReleaseDataFlagOn();
    m_mapper->GlobalWarningDisplayOff();
    m_mapper->AutoAdjustSampleDistancesOn();
    m_mapper->SetScalarModeToUsePointData();
    m_mapper->SetBlendModeToComposite();
    m_mapper->SetMaxMemoryFraction(0.95);
    m_mapper->SetMaxMemoryInBytes(1024 * 1024 * 1024);
    m_mapper->SetInputConnection(m_data->toVTK());
    m_mapper->Update();

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

    m_actor = vtkSmartPointer<vtkVolume>::New();
    m_actor->UseBoundsOn();
    m_actor->PickableOn();
    m_actor->SetMapper(m_mapper);
    m_actor->SetProperty(property);
    m_actor->Update();

    m_view = view;
  }

  //-----------------------------------------------------------------------------
  QList<vtkProp*> VolumeGPURaycastRepresentation::getActors()
  {
    QList<vtkProp*> list;
    list << m_actor.GetPointer();

    return list;
  }

  //-----------------------------------------------------------------------------
  GraphicalRepresentationSPtr VolumeGPURaycastRepresentation::cloneImplementation(VolumeView *view)
  {
    VolumeGPURaycastRepresentation *representation = new VolumeGPURaycastRepresentation(m_data, view);
    representation->initializePipeline(view);

    return GraphicalRepresentationSPtr(representation);
  }

  //-----------------------------------------------------------------------------
  void VolumeGPURaycastRepresentation::updateVisibility(bool visible)
  {
    m_actor->SetVisibility(visible);
  }

} /* namespace EspINA */
