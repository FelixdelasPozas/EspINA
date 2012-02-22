/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  Jorge Peña Pastor <jpena@cesvima.upm.es>

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


#include "regions/BoundingRegion.h"

#include <vtkSMPropertyHelper.h>
#include <vtkSMProxy.h>
#include <pqPipelineSource.h>

//-----------------------------------------------------------------------------
BoundingRegion::BoundingRegion(int left,  int top,    int upper,
			       int right, int bottom, int lower )
: QStandardItem()
{
  m_inclusion[0] = left;  m_exclusion[0] = right;
  m_inclusion[1] = top;   m_exclusion[1] = bottom;
  m_inclusion[2] = upper; m_exclusion[2] = lower;
}

//-----------------------------------------------------------------------------
unsigned int BoundingRegion::totalVolume()
{
  int vol;

  m_boundingRegion->pipelineSource()->updatePipeline();
  vtkSMProxy *proxy = m_boundingRegion->pipelineSource()->getProxy();
  proxy->UpdatePropertyInformation();
  vtkSMPropertyHelper(proxy,"TotalVolume").Get(&vol,1);

  return vol;
}

//-----------------------------------------------------------------------------
unsigned int BoundingRegion::inclusionVolume()
{
  int vol;

  m_boundingRegion->pipelineSource()->updatePipeline();
  vtkSMProxy *proxy = m_boundingRegion->pipelineSource()->getProxy();
  proxy->UpdatePropertyInformation();
  vtkSMPropertyHelper(proxy,"InclusionVolume").Get(&vol,1);

  return vol;
}

//-----------------------------------------------------------------------------
unsigned int BoundingRegion::exclusionVolume()
{
  int vol;

  m_boundingRegion->pipelineSource()->updatePipeline();
  vtkSMProxy *proxy = m_boundingRegion->pipelineSource()->getProxy();
  proxy->UpdatePropertyInformation();
  vtkSMPropertyHelper(proxy,"ExclusionVolume").Get(&vol,1);

  return vol;
}