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


#include "CountingFrames/CountingFrame.h"
#include "vtkCountingFrameSliceWidget.h"
#include "Extensions/CountingFrameExtension.h"
#include <Core/Model/Channel.h>
#include <GUI/ViewManager.h>

using namespace EspINA;

//-----------------------------------------------------------------------------
CountingFrame::CountingFrame(Id                      id,
                             CountingFrameExtension *channelExt,
                             Nm                      inclusion[3],
                             Nm                      exclusion[3],
                             ViewManager            *vm)
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
void CountingFrame::setMargins(Nm inclusion[3], Nm exclusion[3])
{
  memcpy(m_inclusion, inclusion, 3*sizeof(Nm));
  memcpy(m_exclusion, exclusion, 3*sizeof(Nm));

  updateCountingFrame();
}

//-----------------------------------------------------------------------------
void CountingFrame::margins(Nm inclusion[3], Nm exclusion[3])
{
  memcpy(inclusion, m_inclusion, 3*sizeof(Nm));
  memcpy(exclusion, m_exclusion, 3*sizeof(Nm));
}

//-----------------------------------------------------------------------------
QVariant CountingFrame::data(int role) const
{
  if (role == DescriptionRole)
  {
    double spacing[3];
    m_channelExt->channel()->volume()->spacing(spacing);
    Nm voxelVol = spacing[0]*spacing[1]*spacing[2];
    int totalVoxelVolume = totalVolume() /voxelVol;
    int inclusionVoxelVolume = inclusionVolume() / voxelVol;
    int exclusionVoxelVolume = exclusionVolume() / voxelVol;

    QString cube = QString::fromUtf8("\u00b3");
    QString br = "\n";
    QString desc;
    desc += tr("CF:   %1"            ).arg(m_id)                             + br;
    desc += tr("Type: %1"            ).arg(name())                           + br;
    desc += tr("Volume informtation:")                                       + br;
    desc += tr("  Total Volume:"     )                                       + br;
    desc += tr("    %1 voxel"        ).arg(totalVoxelVolume)                 + br;
    desc += tr("    %1 nm"           ).arg(totalVolume(),0,'f',2)     + cube + br;
    desc += tr("  Inclusion Volume:" )                                       + br;
    desc += tr("    %1 voxel"        ).arg(inclusionVoxelVolume)             + br;
    desc += tr("    %1 nm"           ).arg(inclusionVolume(),0,'f',2) + cube + br;
    desc += tr("  Exclusion Volume:" )                                       + br;
    desc += tr("    %1 voxel"        ).arg(exclusionVoxelVolume)             + br;
    desc += tr("    %1 nm"           ).arg(exclusionVolume(),0,'f',2) + cube + br;

    return desc;
  }
  return QStandardItem::data(role);
}

//-----------------------------------------------------------------------------
void CountingFrame::Execute(vtkObject* caller, long unsigned int eventId, void* callData)
{
  vtkCountingFrameSliceWidget *widget = static_cast<vtkCountingFrameSliceWidget *>(caller);

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

    updateCountingFrame();
  }
  //m_viewManager->updateViews();

  emitDataChanged();
}

//-----------------------------------------------------------------------------
void CountingFrame::setTaxonomicalConstraint(const TaxonomyElementPtr taxonomy)
{
  m_taxonomicalConstraint = taxonomy;

  emit modified(this);
}
//-----------------------------------------------------------------------------
void CountingFrame::updateCountingFrame()
{
  updateCountingFrameImplementation();

  foreach(vtkCountingFrameWidget *w, m_widgets2D)
    w->SetCountingFrame(m_representation, m_inclusion, m_exclusion);

  foreach(vtkCountingFrameWidget *w, m_widgets3D)
    w->SetCountingFrame(m_boundingRegion, m_inclusion, m_exclusion);

  emit modified(this);
}
