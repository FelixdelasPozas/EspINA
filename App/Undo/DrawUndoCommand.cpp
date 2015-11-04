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
#include <Core/Analysis/Output.h>
#include <Core/Analysis/Data/Volumetric/SparseVolume.hxx>
#include <Core/Analysis/Data/Volumetric/SparseVolumeUtils.h>
#include <Core/Analysis/Data/Mesh/MarchingCubesMesh.h>
#include <Core/Utils/SignalBlocker.h>
#include <Undo/DrawUndoCommand.h>

using namespace ESPINA;

class UpdateMeshTask
: public Task
{
public:
  explicit UpdateMeshTask(SegmentationAdapterSPtr segmentation,
                          SchedulerSPtr           scheduler)
  : Task(scheduler)
  , m_segmentation(segmentation)
  {
    setDescription(tr("Updating '%1' mesh").arg(segmentation->data(Qt::DisplayRole).toString()));
  }


private:
  virtual void run()
  {
    auto output = m_segmentation->output();

    auto mesh = std::make_shared<MarchingCubesMesh>(output.get());
    output->setData(mesh);

    m_segmentation->invalidateRepresentations();
  }

  SegmentationAdapterSPtr m_segmentation;
};

//-----------------------------------------------------------------------------
DrawUndoCommand::DrawUndoCommand(Support::Context &context,
                                 SegmentationAdapterSPtr seg,
                                 BinaryMaskSPtr<unsigned char> mask)
: WithContext(context)
, m_segmentation(seg)
, m_mask(mask)
, m_image{nullptr}
, m_hasVolume(hasVolumetricData(seg->output()))
, m_updateMesh(std::make_shared<UpdateMeshTask>(seg, context.scheduler()))
{
  if(m_hasVolume)
  {
    auto volume = readLockVolume(seg->output());
    m_bounds = volume->bounds();

    if(intersect(m_bounds, mask->bounds()))
    {
      auto bounds = intersection(m_bounds, mask->bounds());
      m_image  = volume->itkImage(bounds);
    }
  }
  else
  {
    m_bounds = mask->bounds();
    m_image  = mask->itkImage();
  }
}

//-----------------------------------------------------------------------------
void DrawUndoCommand::redo()
{
  auto output = m_segmentation->output();

  if (m_hasVolume)
  {
    auto volume = writeLockVolume(output);
    SignalBlocker<Output::WriteLockData<DefaultVolumetricData>> blockSignals(volume);
    expandAndDraw(volume, m_mask);
  }
  else
  {
    auto strokeSpacing = m_segmentation->output()->spacing();
    auto volume = std::make_shared<SparseVolume<itkVolumeType>>(m_bounds, strokeSpacing);
    volume->draw(m_image);

    output->setData(volume);
  }

  Task::submit(m_updateMesh);
}

//-----------------------------------------------------------------------------
void DrawUndoCommand::undo()
{
  if (m_hasVolume)
  {
    auto volume = writeLockVolume(m_segmentation->output());
    SignalBlocker<Output::WriteLockData<DefaultVolumetricData>> blockSignals(volume);
    volume->resize(m_bounds);
    if(m_image != nullptr)
    {
      volume->draw(m_image);
    }
  }
  else
  {
    m_segmentation->output()->removeData(VolumetricData<itkVolumeType>::TYPE);
  }

  Task::submit(m_updateMesh);
}
