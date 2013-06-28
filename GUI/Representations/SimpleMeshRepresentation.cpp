/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2013 Félix de las Pozas Álvarez <felixdelaspozas@gmail.com>
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

// EspINA
#include "GUI/Representations/SimpleMeshRepresentation.h"
#include "GraphicalRepresentationEmptySettings.h"
#include <GUI/QtWidget/VolumeView.h>
#include <Core/ColorEngines/TransparencySelectionHighlighter.h>

// VTK
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkLookupTable.h>
#include <vtkProperty.h>

using namespace EspINA;


//-----------------------------------------------------------------------------
SimpleMeshRepresentation::SimpleMeshRepresentation(MeshRepresentationSPtr mesh, EspinaRenderView *view)
: IMeshRepresentation(mesh, view)
{
  setLabel(tr("Mesh"));
}

//-----------------------------------------------------------------------------
void SimpleMeshRepresentation::initializePipeline()
{
  connect(m_data.get(), SIGNAL(representationChanged()),
          this, SLOT(updatePipelineConnections()));

  m_mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  m_mapper->ReleaseDataFlagOn();
  m_mapper->ImmediateModeRenderingOn();
  m_mapper->ScalarVisibilityOff();
  m_mapper->SetInputConnection(m_data->mesh());
  m_mapper->Update();

  m_actor = vtkSmartPointer<vtkActor>::New();
  m_actor->SetMapper(m_mapper);
  m_actor->GetProperty()->SetSpecular(0.2);

  LUTPtr colors = s_highlighter->lut(m_color, m_highlight);

  double *rgba = colors->GetTableValue(1);
  m_actor->GetProperty()->SetColor(rgba[0], rgba[1], rgba[2]);
  m_actor->GetProperty()->SetOpacity(rgba[3]);
  m_actor->SetVisibility(isVisible());
  m_actor->Modified();
}

//-----------------------------------------------------------------------------
GraphicalRepresentationSettings *SimpleMeshRepresentation::settingsWidget()
{
  return new GraphicalRepresentationEmptySettings();
}

//-----------------------------------------------------------------------------
void SimpleMeshRepresentation::updateRepresentation()
{
  if (isVisible() && (m_actor != NULL))
  {
    m_mapper->Update();
    m_actor->GetProperty()->Modified();
    m_actor->Modified();
  }
}

//-----------------------------------------------------------------------------
GraphicalRepresentationSPtr SimpleMeshRepresentation::cloneImplementation(VolumeView *view)
{
  SimpleMeshRepresentation *representation = new SimpleMeshRepresentation(m_data, view);
  representation->setView(view);

  return GraphicalRepresentationSPtr(representation);
}

//-----------------------------------------------------------------------------
void SimpleMeshRepresentation::updatePipelineConnections()
{
  if ((m_actor != NULL) && (m_mapper->GetInputConnection(0,0) != m_data->mesh()))
  {
    m_mapper->SetInputConnection(m_data->mesh());
    m_mapper->Update();
  }
}
