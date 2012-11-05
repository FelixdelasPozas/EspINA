/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  <copyright holder> <email>

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


#include "PlanarSplitTool.h"

//-----------------------------------------------------------------------------
PlanarSplitTool::PlanarSplitTool()
: m_inUse(false)
, m_enable(true)
, m_widget(NULL)
{

}

//-----------------------------------------------------------------------------
QCursor PlanarSplitTool::cursor() const
{
  return QCursor(Qt::ArrowCursor);
}

//-----------------------------------------------------------------------------
bool PlanarSplitTool::filterEvent(QEvent* e, EspinaRenderView* view)
{
  if (m_inUse && m_enable && m_widget)
    return m_widget;

  return false;
}

//-----------------------------------------------------------------------------
bool PlanarSplitTool::enabled() const
{
  return m_enable;
}

//-----------------------------------------------------------------------------
void PlanarSplitTool::setEnabled(bool value)
{
  if (m_enable != value)
    m_enable = value;
}

//-----------------------------------------------------------------------------
void PlanarSplitTool::setInUse(bool value)
{
  if (m_inUse != value)
  {
    m_inUse = value;
    if (!m_inUse)
      emit splittingStopped();
  }
}
