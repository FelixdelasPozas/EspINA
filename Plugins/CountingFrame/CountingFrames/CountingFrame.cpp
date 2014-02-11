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
#include <Core/Analysis/Channel.h>
#include <Core/Analysis/Data/VolumetricData.h>
#include <Core/Utils/VolumeBounds.h>

using namespace EspINA;
using namespace EspINA::CF;

//-----------------------------------------------------------------------------
CountingFrame::CountingFrame(CountingFrameExtension *extension,
                             Nm                      inclusion[3],
                             Nm                      exclusion[3])
: INCLUSION_FACE(255)
, EXCLUSION_FACE(0)
, m_visible(true)
, m_highlight(false)
, m_extension(extension)
, m_totalVolume(0)
, m_inclusionVolume(0)
{
  memcpy(m_inclusion, inclusion, 3*sizeof(Nm));
  memcpy(m_exclusion, exclusion, 3*sizeof(Nm));
}

//-----------------------------------------------------------------------------
void CountingFrame::deleteFromExtension()
{
  m_extension->deleteCountingFrame(this);
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
QString CountingFrame::description() const
{
  auto channel  = m_extension->extendedItem();
  auto spacing  = channel->output()->spacing();
  Nm   voxelVol = spacing[0]*spacing[1]*spacing[2];

  int  totalVoxelVolume     = totalVolume()     / voxelVol;
  int  inclusionVoxelVolume = inclusionVolume() / voxelVol;
  int  exclusionVoxelVolume = exclusionVolume() / voxelVol;

  QString cube = QString::fromUtf8("\u00b3");
  QString br = "\n";
  QString desc;
  desc += tr("CF:   %1"            ).arg(m_id)                             + br;
  desc += tr("Type: %1"            ).arg(name())                           + br;
  desc += tr("Volume information:" )                                       + br;
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

//-----------------------------------------------------------------------------
void CountingFrame::setVisible(bool visible)
{
  m_visible = visible;

  for (auto widget2D : m_widgets2D)
  {
    widget2D->SetEnabled(m_visible);
  }

  for (auto widget3D : m_widgets3D)
  {
    widget3D->SetEnabled(m_visible);
  }
}

//-----------------------------------------------------------------------------
void CountingFrame::setHighlighted(bool highlight)
{
  m_highlight = highlight;

  for (auto widget2D : m_widgets2D)
  {
    widget2D->SetHighlighted(m_highlight);
  }

//   for (auto widget3D : m_widgets3D)
//   {
//     widget3D->SetEnabled(m_highlight);
//   }

}


//-----------------------------------------------------------------------------
void CountingFrame::Execute(vtkObject* caller, long unsigned int eventId, void* callData)
{
  auto widget = static_cast<vtkCountingFrameSliceWidget *>(caller);

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
}

//-----------------------------------------------------------------------------
void CountingFrame::setCategoryConstraint(const QString& category)
{
  m_categoryConstraint = category;

  emit modified(this);
}

//-----------------------------------------------------------------------------
void CountingFrame::updateCountingFrame()
{
  updateCountingFrameImplementation();

  for(vtkCountingFrameWidget *widget : m_widgets2D)
  {
    widget->SetCountingFrame(m_representation, m_inclusion, m_exclusion);
  }

  for(vtkCountingFrameWidget *widget : m_widgets3D)
  {
    widget->SetCountingFrame(m_countingFrame, m_inclusion, m_exclusion);
  }

  emit modified(this);
}

//-----------------------------------------------------------------------------
Nm CountingFrame::equivalentVolume(const Bounds& bounds)
{
  auto channel = m_extension->extendedItem();
  auto volume  = volumetricData(channel->output());

  VolumeBounds volumeBounds(bounds, volume->spacing(), volume->origin());

  return (volumeBounds[1]-volumeBounds[0])*(volumeBounds[3]-volumeBounds[2])* (volumeBounds[5]-volumeBounds[4]);
}
