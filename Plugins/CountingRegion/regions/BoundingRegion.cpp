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
#include <extensions/CountingRegionChannelExtension.h>
#include <Core/Model/Channel.h>
#include <GUI/ViewManager.h>

//-----------------------------------------------------------------------------
BoundingRegion::BoundingRegion(RegionId id,
                               CountingRegionChannelExtension *channelExt,
                               Nm inclusion[3],
                               Nm exclusion[3],
                               ViewManager *vm)
: QStandardItem()
, INCLUSION_FACE(255)
, EXCLUSION_FACE(0)
, m_viewManager(vm)
, m_channelExt(channelExt)
, m_id(id)
, m_totalVolume(0)
, m_inclusionVolume(0)
, m_taxonomicalConstraint(NULL)
{
  memcpy(m_inclusion, inclusion, 3*sizeof(Nm));
  memcpy(m_exclusion, exclusion, 3*sizeof(Nm));
}

//-----------------------------------------------------------------------------
void BoundingRegion::setMargins(Nm inclusion[3], Nm exclusion[3])
{
  memcpy(m_inclusion, inclusion, 3*sizeof(Nm));
  memcpy(m_exclusion, exclusion, 3*sizeof(Nm));

  updateBoundingRegion();
}

//-----------------------------------------------------------------------------
QVariant BoundingRegion::data(int role) const
{
  if (role == DescriptionRole)
  {
    double spacing[3];
    m_channelExt->channel()->volume()->spacing(spacing);
    Nm voxelVol = spacing[0]*spacing[1]*spacing[2];
    int totalVoxelVolume = totalVolume() /voxelVol;
    int inclusionVoxelVolume = inclusionVolume() / voxelVol;
    int exclusionVoxelVolume = exclusionVolume() / voxelVol;

    QString sqr = QString::fromUtf8("\u00b2");
    QString br = "\n";
    QString desc;
    desc += tr("Region:  %1"         ).arg(m_id)                            + br;
    desc += tr("Type: %1"            ).arg(regionType())                    + br;
    desc += tr("Volume informtation:")                                      + br;
    desc += tr("  Total Volume:"     )                                      + br;
    desc += tr("    %1 voxel"        ).arg(totalVoxelVolume)                + br;
    desc += tr("    %1 nm"           ).arg(totalVolume(),0,'f',2)     + sqr + br;
    desc += tr("  Inclusion Volume:" )                                      + br;
    desc += tr("    %1 voxel"        ).arg(inclusionVoxelVolume)            + br;
    desc += tr("    %1 nm"           ).arg(inclusionVolume(),0,'f',2) + sqr + br;
    desc += tr("  Exclusion Volume:" )                                      + br;
    desc += tr("    %1 voxel"        ).arg(exclusionVoxelVolume)            + br;
    desc += tr("    %1 nm"           ).arg(exclusionVolume(),0,'f',2) + sqr + br;

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
      m_inclusion[i] = inOffset[i];
      if (m_inclusion[i] < 0)
        m_inclusion[i] = 0;

      m_exclusion[i] = exOffset[i];
      if (m_exclusion[i] < 0)
        m_exclusion[i] = 0;
    }

    updateBoundingRegion();
  }
  //m_viewManager->updateViews();

  emitDataChanged();
}

//-----------------------------------------------------------------------------
void BoundingRegion::setTaxonomicalConstraint(const TaxonomyElement* taxonomy)
{
  m_taxonomicalConstraint = taxonomy;

  emit modified(this);
}
//-----------------------------------------------------------------------------
void BoundingRegion::updateBoundingRegion()
{
  updateBoundingRegionImplementation();

  foreach(vtkBoundingRegionWidget *w, m_widgets2D)
    w->SetBoundingRegion(m_representation, m_inclusion, m_exclusion);

  foreach(vtkBoundingRegionWidget *w, m_widgets3D)
    w->SetBoundingRegion(m_boundingRegion, m_inclusion, m_exclusion);

  emit modified(this);
}
