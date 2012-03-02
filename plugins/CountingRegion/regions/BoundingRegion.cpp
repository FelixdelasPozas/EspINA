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
BoundingRegion::BoundingRegion(double inclusion[3], double exclusion[3])
: QStandardItem()
{
  memcpy(m_inclusion, inclusion, 3*sizeof(double));
  memcpy(m_exclusion, exclusion, 3*sizeof(double));
}

//-----------------------------------------------------------------------------
QVariant BoundingRegion::data(int role) const
{
  if (role == DescriptionRole)
  {
    QString desc("Type: Rectangular Region\n"
    "Volume Information:\n"
    "  Total Volume:\n"
    "    %1 px\n"
    "    %2 %3\n"
    "  Inclusion Volume:\n"
    "    %4 px\n"
    "    %5 %3\n"
    "  Exclusion Volume:\n"
    "    %6 px\n"
    "    %7 %3\n"
    );

    double totalPixelVolume = totalVolume();// /volPixel;
    double inclusionPixelVolume = inclusionVolume();// /volPixel;
    double exclusionPixelVolume = exclusionVolume();// /volPixel;
    desc = desc.arg(totalPixelVolume,0).arg(totalVolume(),0,'f',2).arg("nm");
    desc = desc.arg(inclusionPixelVolume,0).arg(inclusionVolume(),0,'f',2);
    desc = desc.arg(exclusionPixelVolume,0).arg(exclusionVolume(),0,'f',2);

    return desc;
  }
  return QStandardItem::data(role);
}

//-----------------------------------------------------------------------------
double BoundingRegion::totalVolume() const
{
  double vol;

  m_boundingRegion->pipelineSource()->updatePipeline();
  vtkSMProxy *proxy = m_boundingRegion->pipelineSource()->getProxy();
  proxy->UpdatePropertyInformation();
  vtkSMPropertyHelper(proxy,"TotalVolume").Get(&vol,1);

  return vol;
}

//-----------------------------------------------------------------------------
double BoundingRegion::inclusionVolume() const
{
  double vol;

  m_boundingRegion->pipelineSource()->updatePipeline();
  vtkSMProxy *proxy = m_boundingRegion->pipelineSource()->getProxy();
  proxy->UpdatePropertyInformation();
  vtkSMPropertyHelper(proxy,"InclusionVolume").Get(&vol,1);

  return vol;
}

//-----------------------------------------------------------------------------
double BoundingRegion::exclusionVolume() const
{
  double vol;

  m_boundingRegion->pipelineSource()->updatePipeline();
  vtkSMProxy *proxy = m_boundingRegion->pipelineSource()->getProxy();
  proxy->UpdatePropertyInformation();
  vtkSMPropertyHelper(proxy,"ExclusionVolume").Get(&vol,1);

  return vol;
}