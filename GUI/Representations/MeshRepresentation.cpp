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
#include "GUI/Representations/MeshRepresentation.h"
#include "GUI/QtWidget/VolumeView.h"
#include <Core/ColorEngines/TransparencySelectionHighlighter.h>

// VTK
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkLookupTable.h>
#include <vtkProperty.h>

namespace EspINA
{
  TransparencySelectionHighlighter *MeshRepresentation::s_highlighter = new TransparencySelectionHighlighter();

  //-----------------------------------------------------------------------------
  MeshRepresentation::MeshRepresentation(MeshTypeSPtr mesh, EspinaRenderView *view)
  : SegmentationGraphicalRepresentation(view)
  , m_data(mesh)
  {
  }

  //-----------------------------------------------------------------------------
  MeshRepresentation::~MeshRepresentation()
  {
    if (m_view)
      m_view->removeActor(m_actor);
  }

  //-----------------------------------------------------------------------------
  void MeshRepresentation::setColor(const QColor &color)
  {
    SegmentationGraphicalRepresentation::setColor(color);

    LUTPtr colors = s_highlighter->lut(m_color, m_highlight);
    
    double *rgba = colors->GetTableValue(1);
    m_actor->GetProperty()->SetColor(rgba[0], rgba[1], rgba[2]);
    m_actor->GetProperty()->SetOpacity(rgba[3]);
  }
  
  //-----------------------------------------------------------------------------
  void MeshRepresentation::setHighlighted(bool highlighted)
  {
    GraphicalRepresentation::setHighlighted(highlighted);

    LUTPtr colors = s_highlighter->lut(m_color, m_highlight);

    double *rgba = colors->GetTableValue(1);
    m_actor->GetProperty()->SetColor(rgba[0], rgba[1], rgba[2]);
    m_actor->GetProperty()->SetOpacity(rgba[3]);
  }

  //-----------------------------------------------------------------------------
  void MeshRepresentation::setVisible(bool visible)
  {
    SegmentationGraphicalRepresentation::setVisible(visible);

    m_actor->SetVisibility(m_visible);
  }

  //-----------------------------------------------------------------------------
  bool MeshRepresentation::hasActor(vtkProp *actor) const
  {
    return m_actor.GetPointer() == actor;
  }

  //-----------------------------------------------------------------------------
  bool MeshRepresentation::isInside(Nm *point)
  {
    // FIXME: unused now, buy maybe useful in the future
    return false;
  }

  //-----------------------------------------------------------------------------
  GraphicalRepresentationSPtr MeshRepresentation::clone(VolumeView *view)
  {
    MeshRepresentation *representation = new MeshRepresentation(m_data, view);
    representation->initializePipeline(view);

    return GraphicalRepresentationSPtr(representation);
  }

  //-----------------------------------------------------------------------------
  void MeshRepresentation::initializePipeline(VolumeView *view)
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
    m_actor->Modified();

    view->addActor(m_actor);
    m_view = view;
  }

  //-----------------------------------------------------------------------------
  void MeshRepresentation::updateRepresentation()
  {
    m_mapper->Update();
    m_actor->GetProperty()->Modified();
    m_actor->Modified();
  }

  //-----------------------------------------------------------------------------
  void MeshRepresentation::updatePipelineConnections()
  {
    if (m_mapper->GetInputConnection(0,0) != m_data->mesh())
    {
      m_mapper->SetInputConnection(m_data->mesh());
      m_mapper->Update();
    }
  }

} /* namespace EspINA */
