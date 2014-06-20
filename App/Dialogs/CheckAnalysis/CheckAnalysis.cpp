/*
 <one line to give the program's name and a brief idea of what it does.>
 Copyright (C) 2014 Félix de las Pozas Álvarez <fpozas@cesvima.upm.es>

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

// EspINA
#include "CheckAnalysis.h"
#include <Core/Analysis/Data/VolumetricData.h>
#include <Core/Analysis/Data/MeshData.h>
#include <Core/Analysis/Sample.h>
#include <Core/Analysis/Channel.h>
#include <GUI/Model/Utils/QueryAdapter.h>


namespace EspINA
{
  //------------------------------------------------------------------------
  CheckAnalysis::CheckAnalysis(SchedulerSPtr scheduler, ModelAdapterSPtr model)
  : m_problemsNum{0}
  {
    setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowCloseButtonHint);
    qRegisterMetaType<struct Problem>("struct Problem");

    m_progress->setMinimum(0);
    m_progress->setMaximum(model->segmentations().size() + model->channels().size() + model->samples().size());
    m_progress->setValue(0);

    for(auto seg: model->segmentations())
    {
      Task *task = new CheckTask(scheduler, seg, model);
      m_taskList << TaskSPtr(task);
    }

    for(auto channel: model->channels())
    {
      Task *task = new CheckTask(scheduler, channel, model);
      m_taskList << TaskSPtr(task);
    }

    for(auto sample: model->samples())
    {
      Task *task = new CheckTask(scheduler, sample, model);
      m_taskList << TaskSPtr(task);
    }

    for(auto task: m_taskList)
    {
      connect(task.get(), SIGNAL(finished()), this, SLOT(finishedTask()), Qt::QueuedConnection);
      connect(task.get(), SIGNAL(problem(struct Problem)), this, SLOT(addProblem(struct Problem)), Qt::QueuedConnection);
      task->submit(task);
    }
  }
  
  //------------------------------------------------------------------------
  void CheckAnalysis::finishedTask()
  {
    auto senderTask = qobject_cast<CheckTask *>(sender());
    Q_ASSERT(senderTask);

    m_progress->setValue(m_progress->value()+1);

    for(auto task: m_taskList)
      if(task.get() == senderTask)
      {
        disconnect(task.get(), SIGNAL(finished()), this, SLOT(finishedTask()));
        disconnect(task.get(), SIGNAL(problem(struct Problem)), this, SLOT(addProblem(struct Problem)));
        m_taskList.removeOne(task);
        break;
      }

    if(m_taskList.empty())
      close();
  }

  //------------------------------------------------------------------------
  void CheckAnalysis::addProblem(struct Problem problem)
  {
    m_problems << problem;
    m_numProblems->setText(QString().number(++m_problemsNum));
  }

  //------------------------------------------------------------------------
  void CheckTask::run()
  {
    switch(m_item->type())
    {
      case ItemAdapter::Type::SEGMENTATION:
        checkViewItemOutputs();
        checkSegmentationHasChannel();
        checkSegmentationRelations();
        break;
      case ItemAdapter::Type::CHANNEL:
        checkViewItemOutputs();
        checkChannelRelations();
        break;
      case ItemAdapter::Type::SAMPLE:
        break;
      default:
        break;
    }
  }

  //------------------------------------------------------------------------
  void CheckTask::checkVolumeIsEmpty() const
  {
    switch(m_item->type())
    {
      case ItemAdapter::Type::SEGMENTATION:
        {
          auto seg = std::dynamic_pointer_cast<SegmentationAdapter>(m_item);
          auto volume = volumetricData(seg->output());
          if(volume == nullptr || volume->isEmpty())
          {
            struct Problem segProblem{seg->data().toString(), Severity::CRITICAL, "Segmentation volume is empty or null.", "Delete segmentation."};
            emit problem(segProblem);
          }
        }
        break;
      case ItemAdapter::Type::CHANNEL:
        {
          auto channel = std::dynamic_pointer_cast<ChannelAdapter>(m_item);
          auto volume = volumetricData(channel->output());
          if(volume == nullptr)
          {
            struct Problem channelProblem{channel->data().toString(), Severity::CRITICAL, "Channel volume is null.", "Delete channel."};
            emit problem(channelProblem);
          }
        }
        break;
      default:
        Q_ASSERT(false);
        break;
    }
  }

  //------------------------------------------------------------------------
  void CheckTask::checkMeshIsEmpty() const
  {
    auto seg = std::dynamic_pointer_cast<SegmentationAdapter>(m_item);
    auto mesh = meshData(seg->output());

    if(mesh == nullptr || mesh->mesh() == nullptr || mesh->mesh()->GetNumberOfPoints() == 0)
    {
      struct Problem segProblem{seg->data().toString(), Severity::CRITICAL, "Segmentation mesh is empty or null.", "Delete segmentation"};
      emit problem(segProblem);
    }
  }

  //------------------------------------------------------------------------
  void CheckTask::checkSegmentationHasChannel() const
  {
    auto seg = std::dynamic_pointer_cast<SegmentationAdapter>(m_item);
    auto channels = QueryAdapter::channels(seg);

    if(channels.empty())
    {
      struct Problem segProblem{seg->data().toString(), Severity::CRITICAL, "Segmentation is not related to any channel.", "Delete segmentation."};
      emit problem(segProblem);
    }
  }

  //------------------------------------------------------------------------
  void CheckTask::checkSegmentationRelations() const
  {
    auto seg = std::dynamic_pointer_cast<SegmentationAdapter>(m_item);
    auto relations = m_model->relations(seg.get(), RelationType::RELATION_INOUT, Sample::CONTAINS);

    if(relations.empty())
    {
      struct Problem segProblem{seg->data().toString(), Severity::CRITICAL, "Segmentation is not related to any sample.", "Delete segmentation."};
      emit problem(segProblem);
    }
  }

  //------------------------------------------------------------------------
  void CheckTask::checkChannelRelations() const
  {
    auto channel = std::dynamic_pointer_cast<ChannelAdapter>(m_item);
    auto relations = m_model->relations(channel.get(), RelationType::RELATION_INOUT, Channel::STAIN_LINK);

    if(relations.empty())
    {
      struct Problem segProblem{channel->data().toString(), Severity::CRITICAL, "Channel is not related to any sample.", "Change relations in the \"Channel Explorer\" dialog."};
      emit problem(segProblem);
    }
  }

  //------------------------------------------------------------------------
  void CheckTask::checkViewItemOutputs() const
  {
    auto viewItem = std::dynamic_pointer_cast<ViewItemAdapter>(m_item);
    auto output = viewItem->output();
    auto filter = viewItem->filter();

    if (output == nullptr)
    {
      struct Problem segProblem { viewItem->data().toString(), Severity::CRITICAL, "Item does not have an output.", "Delete item." };
      emit problem(segProblem);
    }
    else
    {
      int numberOfDatas = 0;
      if (output->hasData(MeshData::TYPE))
      {
        checkMeshIsEmpty();
        ++numberOfDatas;
      }

      if (output->hasData(VolumetricData<itkVolumeType>::TYPE))
      {
        checkVolumeIsEmpty();
        ++numberOfDatas;
      }

      if (numberOfDatas == 0)
      {
        struct Problem segProblem { viewItem->data().toString(), Severity::CRITICAL, "Item does not have any data.", "Delete item." };
        emit problem(segProblem);
      }
    }

    if (filter == nullptr)
    {
      struct Problem segProblem { viewItem->data().toString(), Severity::CRITICAL, "Item does not have a filter.", "Delete item." };
      emit problem(segProblem);
    }

    // TODO: fix output if filter != nullptr, requires model??
  }

} // namespace EspINA
