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
#include "vtkBoundingRegionSliceWidget.h"

//-----------------------------------------------------------------------------
BoundingRegion::BoundingRegion(CountingRegionSampleExtension *sampleExt,
			       double inclusion[3],
			       double exclusion[3])
: QStandardItem()
, INCLUSION_FACE(255)
, EXCLUSION_FACE(0)
, m_sampleExt(sampleExt)
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
void BoundingRegion::Execute(vtkObject* caller, long unsigned int eventId, void* callData)
{
  vtkBoundingRegionSliceWidget *widget = static_cast<vtkBoundingRegionSliceWidget *>(caller);

  if (widget)
  {
    Nm inOffset[3], exOffset[3];
    widget->GetInclusionOffset(inOffset);
    widget->GetExclusionOffset(exOffset);
    for (int i = 0; i < 3; i++)
    {
      m_inclusion[i] += inOffset[i];
      m_exclusion[i] += exOffset[i];
    }

    updateBoundingRegion();

    foreach(vtkBoundingRegionWidget *w, m_widgets)
      w->SetBoundingRegion(m_boundingRegion);
  }

  emitDataChanged();
}
