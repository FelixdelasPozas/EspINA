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
#include "RulerSliceWidget.h"
#include "vtkRulerWidget.h"

// VTK
#include <vtkAbstractWidget.h>


namespace EspINA
{
  //----------------------------------------------------------------------------
  RulerSliceWidget::RulerSliceWidget(vtkAbstractWidget *widget)
  : SliceWidget(widget)
  , m_pos(-1)
  , m_plane(EspINA::AXIAL)
  , m_insideBounds(false)
  , m_enabled(false)
  {
  }

  //----------------------------------------------------------------------------
  RulerSliceWidget::~RulerSliceWidget()
  {
  }

  //----------------------------------------------------------------------------
  void RulerSliceWidget::setEnabled(int value)
  {
    m_enabled = value;
  }

  //----------------------------------------------------------------------------
  void RulerSliceWidget::setSlice(Nm pos, PlaneType plane)
  {
    m_pos   = pos;
    m_plane = plane;

    double bounds[6];
    static_cast<vtkRulerWidget *>(m_widget)->bounds(bounds);
    m_insideBounds = bounds[2*plane] <= pos && pos <= bounds[2*plane+1];
    m_widget->SetEnabled(m_insideBounds);
  }

  //----------------------------------------------------------------------------
  void RulerSliceWidget::setBounds(Nm *bounds)
  {
    if (m_enabled)
    {
      static_cast<vtkRulerWidget *>(m_widget)->setBounds(bounds);
      setSlice(m_pos, m_plane);
    }
  }
} /* namespace EspINA */