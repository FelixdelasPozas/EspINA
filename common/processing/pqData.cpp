/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2011  Jorge Peña Pastor<jpena@cesvima.upm.es>

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
#include "pqData.h"

// Debug

// ESPINA
#include "common/cache/CachedObjectBuilder.h"
#include <vtkImageAlgorithm.h>

using namespace std;


///-----------------------------------------------------------------------------
/// pqData
///----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
pqData::pqData(pqFilter* source, unsigned int portNumber)
: m_source(source)
, m_portNumber(portNumber)
{
  Q_ASSERT(portNumber <= source->getNumberOfData());
}

//-----------------------------------------------------------------------------
QString pqData::id() const
{
  return QString("%1:%2").arg(m_source->id()).arg(m_portNumber);
}

//-----------------------------------------------------------------------------
vtkImageAlgorithm *pqData::algorithm()
{
  return m_source->algorithm();
}

//-----------------------------------------------------------------------------
vtkAlgorithmOutput *pqData::outputPort() const
{
  return m_source->algorithm()->GetOutputPort();
}
