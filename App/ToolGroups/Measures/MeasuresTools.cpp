/*
 <one line to give the program's name and a brief idea of what it does.>
 Copyright (C) 2014 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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
#include <ToolGroups/Measures/MeasuresTools.h>

namespace EspINA
{
  //----------------------------------------------------------------------------
  MeasuresTools::MeasuresTools(ViewManagerSPtr viewManager, QWidget* parent)
  : ToolGroup(viewManager, QIcon(":/espina/measure3D.png"), tr("Measure Tools"), parent)
  , m_measure{ new MeasureTool(viewManager) }
  , m_ruler{ new RulerTool(viewManager) }
  , m_enabled{false}
  {
  }
  
  //----------------------------------------------------------------------------
  MeasuresTools::~MeasuresTools()
  {
  }

  //----------------------------------------------------------------------------
  void MeasuresTools::setEnabled(bool value)
  {
    m_enabled = value;

    m_measure->setEnabled(value);
    m_ruler->setEnabled(value);
  }
  
  //----------------------------------------------------------------------------
  bool MeasuresTools::enabled() const
  {
    return m_enabled;
  }
  
  //----------------------------------------------------------------------------
  ToolSList MeasuresTools::tools()
  {
    ToolSList tools;

    tools << m_measure;
    tools << m_ruler;

    return tools;
  }

} /* namespace EspINA */
