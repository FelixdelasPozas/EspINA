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
#include "SmoothedMeshRepresentation.h"
#include "GUI/QtWidget/EspinaRenderView.h"
#include "GUI/QtWidget/VolumeView.h"
#include "Core/ColorEngines/IColorEngine.h"
#include <Core/ColorEngines/TransparencySelectionHighlighter.h>

// VTK
#include <vtkDecimatePro.h>
#include <vtkWindowedSincPolyDataFilter.h>
#include <vtkPolyDataNormals.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkProperty.h>

namespace EspINA
{
  //-----------------------------------------------------------------------------
  SmoothedMeshRepresentation::SmoothedMeshRepresentation(MeshRepresentationSPtr mesh, EspinaRenderView *view)
  : IMeshRepresentation(mesh, view)
  {
  }

  //-----------------------------------------------------------------------------
  void SmoothedMeshRepresentation::initializePipeline(VolumeView *view)
  {
    connect(m_data.get(), SIGNAL(representationChanged()),
            this, SLOT(updatePipelineConnections()));

    m_decimate = vtkSmartPointer<vtkDecimatePro>::New();
    m_decimate->ReleaseDataFlagOn();
    m_decimate->SetGlobalWarningDisplay(false);
    m_decimate->SetTargetReduction(0.95);
    m_decimate->PreserveTopologyOn();
    m_decimate->SplittingOff();
    m_decimate->SetInputConnection(m_data->mesh());

    vtkSmartPointer<vtkWindowedSincPolyDataFilter> smoother = vtkSmartPointer<vtkWindowedSincPolyDataFilter>::New();
    smoother->ReleaseDataFlagOn();
    smoother->SetGlobalWarningDisplay(false);
    smoother->BoundarySmoothingOn();
    smoother->FeatureEdgeSmoothingOn();
    smoother->SetNumberOfIterations(15);
    smoother->SetFeatureAngle(120);
    smoother->SetEdgeAngle(90);
    smoother->SetInputConnection(m_decimate->GetOutputPort());

    vtkSmartPointer<vtkPolyDataNormals> normals = vtkSmartPointer<vtkPolyDataNormals>::New();
    normals->ReleaseDataFlagOn();
    normals->SetFeatureAngle(120);
    normals->SetInputConnection(smoother->GetOutputPort());

    m_mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    m_mapper->ReleaseDataFlagOn();
    m_mapper->ImmediateModeRenderingOn();
    m_mapper->ScalarVisibilityOff();
    m_mapper->SetInputConnection(normals->GetOutputPort());
    m_mapper->Update();

    m_actor = vtkSmartPointer<vtkActor>::New();
    m_actor->SetMapper(m_mapper);
    m_actor->GetProperty()->SetSpecular(0.2);

    LUTPtr colors = s_highlighter->lut(m_color, m_highlight);

    double *rgba = colors->GetTableValue(1);
    m_actor->GetProperty()->SetColor(rgba[0], rgba[1], rgba[2]);
    m_actor->GetProperty()->SetOpacity(rgba[3]);
    m_actor->Modified();

    m_view = view;
  }

  //-----------------------------------------------------------------------------
  void SmoothedMeshRepresentation::updateRepresentation()
  {
    if (isVisible())
    {
      m_decimate->Update();
      m_mapper->Update();
      m_actor->GetProperty()->Modified();
      m_actor->Modified();
    }
  }

  //-----------------------------------------------------------------------------
  GraphicalRepresentationSPtr SmoothedMeshRepresentation::cloneImplementation(VolumeView *view)
  {
    SmoothedMeshRepresentation *representation = new SmoothedMeshRepresentation(m_data, view);
    representation->initializePipeline(view);

    return GraphicalRepresentationSPtr(representation);
  }

  //-----------------------------------------------------------------------------
  void SmoothedMeshRepresentation::updatePipelineConnections()
  {
    if (m_decimate->GetInputConnection(0, 0) != m_data->mesh())
    {
      m_decimate->SetInputConnection(m_data->mesh());
      m_decimate->Update();
    }
  }

} /* namespace EspINA */
