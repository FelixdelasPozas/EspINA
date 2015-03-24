/*
 * 
 * Copyright (C) 2014 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>
 * 
 * This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
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

// ESPINA
#include <Deprecated/GUI/Representations/MeshRepresentation.h>
#include <Deprecated/GUI/Representations/RepresentationEmptySettings.h>
#include <GUI/View/View3D.h>
#include <GUI/ColorEngines/TransparencySelectionHighlighter.h>

// VTK
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkLookupTable.h>
#include <vtkProperty.h>

using namespace ESPINA;

const Representation::Type MeshRepresentation::TYPE = "Mesh";

//-----------------------------------------------------------------------------
MeshRepresentation::MeshRepresentation(MeshDataSPtr mesh, RenderView *view)
: MeshRepresentationBase{mesh, view}
{
  setType(TYPE);
}

//-----------------------------------------------------------------------------
void MeshRepresentation::initializePipeline()
{
  m_mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  m_mapper->ReleaseDataFlagOn();
  m_mapper->ImmediateModeRenderingOn();
  m_mapper->ScalarVisibilityOff();
  m_mapper->SetInputData(m_data->mesh());
  m_mapper->Update();

  m_actor = vtkSmartPointer<vtkActor>::New();
  m_actor->SetMapper(m_mapper);
  m_actor->GetProperty()->SetSpecular(0.2);

  auto colors = s_highlighter->lut(m_color, m_highlight);
  double *rgba = colors->GetTableValue(1);
  m_actor->GetProperty()->SetColor(rgba[0], rgba[1], rgba[2]);
  m_actor->GetProperty()->SetOpacity(rgba[3]);
  m_actor->SetVisibility(isVisible());
  m_actor->Modified();

  m_lastUpdatedTime = m_data->lastModified();
}

//-----------------------------------------------------------------------------
RepresentationSettings *MeshRepresentation::settingsWidget()
{
  return new RepresentationEmptySettings();
}

//-----------------------------------------------------------------------------
void MeshRepresentation::updateRepresentation()
{
  if (isVisible() && (m_actor != nullptr) && needUpdate())
  {
    m_mapper->UpdateWholeExtent();
    m_actor->GetProperty()->Modified();
    m_actor->Modified();

    m_lastUpdatedTime = m_data->lastModified();
  }
}

//-----------------------------------------------------------------------------
RepresentationSPtr MeshRepresentation::cloneImplementation(View3D *view)
{
  auto representation = new MeshRepresentation(m_data, view);
  representation->setView(view);

  return RepresentationSPtr(representation);
}
