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
#include "GUI/Representations/MeshRepresentationBase.h"
#include <GUI/ColorEngines/TransparencySelectionHighlighter.h>
#include "GUI/View/RenderView.h"

// VTK
#include <vtkActor.h>
#include <vtkProperty.h>

using namespace EspINA;

TransparencySelectionHighlighter *MeshRepresentationBase::s_highlighter = new TransparencySelectionHighlighter();

//-----------------------------------------------------------------------------
MeshRepresentationBase::MeshRepresentationBase(MeshDataSPtr mesh, RenderView *view)
: Representation(view)
, m_data(mesh)
{
}

//-----------------------------------------------------------------------------
void MeshRepresentationBase::setColor(const QColor &color)
{
  Representation::setColor(color);

  if (m_actor != nullptr)
  {
    LUTSPtr colors = s_highlighter->lut(m_color, m_highlight);
    double *rgba = colors->GetTableValue(1);
    m_actor->GetProperty()->SetColor(rgba[0], rgba[1], rgba[2]);
    m_actor->GetProperty()->SetOpacity(rgba[3]);
  }
}

//-----------------------------------------------------------------------------
void MeshRepresentationBase::setHighlighted(bool highlighted)
{
  Representation::setHighlighted(highlighted);

  if (m_actor != nullptr)
  {
    LUTSPtr colors = s_highlighter->lut(m_color, m_highlight);
    double *rgba = colors->GetTableValue(1);
    m_actor->GetProperty()->SetColor(rgba[0], rgba[1], rgba[2]);
    m_actor->GetProperty()->SetOpacity(rgba[3]);
  }
}

//-----------------------------------------------------------------------------
bool MeshRepresentationBase::hasActor(vtkProp *actor) const
{
  if (m_actor == nullptr)
    return false;

  return m_actor.GetPointer() == actor;
}

//-----------------------------------------------------------------------------
bool MeshRepresentationBase::isInside(const NmVector3 &point) const
{
  // FIXME: unused now, buy maybe useful in the future
  return false;
}

//-----------------------------------------------------------------------------
QList<vtkProp *> MeshRepresentationBase::getActors()
{
  QList<vtkProp *> list;

  if (m_actor == nullptr)
    initializePipeline();

  list << m_actor.GetPointer();
  return list;
}
//-----------------------------------------------------------------------------
void MeshRepresentationBase::updateVisibility(bool visible)
{
  if (m_actor != nullptr)
    m_actor->SetVisibility(visible);
}
