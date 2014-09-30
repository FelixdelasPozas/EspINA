/*

    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>

    This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
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

// ESPINA
#include "Core/Analysis/Sample.h"

using namespace ESPINA;

const RelationName Sample::CONTAINS = "Contains";

//------------------------------------------------------------------------
Sample::Sample(const QString& name)
{
  setName(name);
}

//------------------------------------------------------------------------
Sample::~Sample()
{
}

//------------------------------------------------------------------------
void Sample::restoreState(const State& state)
{
}

//------------------------------------------------------------------------
State Sample::state() const
{
  return State();
}

//------------------------------------------------------------------------
Snapshot Sample::snapshot() const
{
  return Snapshot();
}

//------------------------------------------------------------------------
void Sample::unload()
{
}

//------------------------------------------------------------------------
void Sample::setPosition(const NmVector3& point)
{
  for(int i = 0; i < 3; ++i)
  {
    Nm d{m_bounds[2*i+1] - m_bounds[2*i]};

    m_bounds[2*i]   = point[i];
    m_bounds[2*i+1] = point[i] + d;
  }
}

//------------------------------------------------------------------------
NmVector3 Sample::position() const
{
  NmVector3 pos;

  for(int i = 0; i < 3; ++i)
  {
    pos[i] = m_bounds[2*i];
  }

  return pos;
}
