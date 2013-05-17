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
#include "GUI/Representations/IMeshRepresentation.h"
#include <Core/ColorEngines/TransparencySelectionHighlighter.h>
#include "GUI/QtWidget/EspinaRenderView.h"

// VTK
#include <vtkActor.h>
#include <vtkProperty.h>

namespace EspINA
{
  TransparencySelectionHighlighter *IMeshRepresentation::s_highlighter = new TransparencySelectionHighlighter();

  //-----------------------------------------------------------------------------
  IMeshRepresentation::IMeshRepresentation(MeshRepresentationSPtr mesh, EspinaRenderView *view)
  : SegmentationGraphicalRepresentation(view)
  , m_data(mesh)
  {
  }
  
  //-----------------------------------------------------------------------------
  IMeshRepresentation::~IMeshRepresentation()
  {
    if (m_view)
      m_view->removeActor(m_actor);
  }

  //-----------------------------------------------------------------------------
  void IMeshRepresentation::setColor(const QColor &color)
  {
    SegmentationGraphicalRepresentation::setColor(color);

    LUTPtr colors = s_highlighter->lut(m_color, m_highlight);

    double *rgba = colors->GetTableValue(1);
    m_actor->GetProperty()->SetColor(rgba[0], rgba[1], rgba[2]);
    m_actor->GetProperty()->SetOpacity(rgba[3]);
  }

  //-----------------------------------------------------------------------------
  void IMeshRepresentation::setHighlighted(bool highlighted)
  {
    GraphicalRepresentation::setHighlighted(highlighted);

    LUTPtr colors = s_highlighter->lut(m_color, m_highlight);

    double *rgba = colors->GetTableValue(1);
    m_actor->GetProperty()->SetColor(rgba[0], rgba[1], rgba[2]);
    m_actor->GetProperty()->SetOpacity(rgba[3]);
  }

  //-----------------------------------------------------------------------------
  void IMeshRepresentation::setVisible(bool visible)
  {
    SegmentationGraphicalRepresentation::setVisible(visible);

    m_actor->SetVisibility(m_visible);
  }

  //-----------------------------------------------------------------------------
  bool IMeshRepresentation::hasActor(vtkProp *actor) const
  {
    return m_actor.GetPointer() == actor;
  }

  //-----------------------------------------------------------------------------
  bool IMeshRepresentation::isInside(Nm *point)
  {
    // FIXME: unused now, buy maybe useful in the future
    return false;
  }
} /* namespace EspINA */
