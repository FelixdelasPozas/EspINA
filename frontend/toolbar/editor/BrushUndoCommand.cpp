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


#include "BrushUndoCommand.h"

#include "common/model/Filter.h"

//-----------------------------------------------------------------------------
Brush::DrawCommand::DrawCommand(Filter* source,
                                QList<vtkImplicitFunction *> brushes,
                                double bounds[6],
                                EspinaVolume::PixelType value)
: m_source(source)
, m_brushes(brushes)
, m_value(value)
{
  memcpy(m_bounds, bounds, 6*sizeof(double));
}

//-----------------------------------------------------------------------------
void Brush::DrawCommand::redo()
{
  m_source->draw(0, m_brushes, m_bounds, m_value);
}


//-----------------------------------------------------------------------------
void Brush::DrawCommand::undo()
{
//     m_source->draw(0, m_brushPointers, m_bounds, SEG_VOXEL_VALUE);
}
