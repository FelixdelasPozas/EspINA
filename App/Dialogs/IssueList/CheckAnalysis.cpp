/*

 Copyright (C) 2014 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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
#include <Core/Analysis/Data/VolumetricData.hxx>
#include <Core/Analysis/Data/MeshData.h>
#include <Core/Analysis/Sample.h>
#include <Core/Analysis/Channel.h>
#include <Dialogs/IssueList/CheckAnalysis.h>
#include <GUI/Model/Utils/QueryAdapter.h>

namespace ESPINA
{
  //------------------------------------------------------------------------
  CheckAnalysis::CheckAnalysis(SchedulerSPtr scheduler, ModelAdapterSPtr model)
  : Task           {scheduler}
  , m_issuesNum    {0}
  , m_finishedTasks{0}
  {
    setDescription(tr("Issues checker"));
    setPriority(Priority::LOW);

    qRegisterMetaType<Issue>("Issue");

    for(auto seg: model->segmentations())
    {
      m_taskList << std::make_shared<CheckSegmentationTask>(scheduler, seg, model);
    }

    for(auto channel: model->channels())
    {
      m_taskList << std::make_shared<CheckChannelTask>(scheduler, channel, model);
    }

    for(auto sample: model->samples())
    {
      m_taskList << std::make_shared<CheckSampleTask>(scheduler, sample, model);
    }
  }

  //------------------------------------------------------------------------
  CheckAnalysis::~CheckAnalysis()
  {
  }

  //------------------------------------------------------------------------
  void CheckAnalysis::run()
  {
    for(auto task: m_taskList)
    {
      connect(task.get(), SIGNAL(finished()),
              this,       SLOT(finishedTask()), Qt::DirectConnection);
      connect(task.get(), SIGNAL(issue(Issue)),
              this,       SLOT(addIssue(Issue)), Qt::DirectConnection);

      task->submit(task);
    }

    pause();
    canExecute(); // stops this thread until the other have finished.
  }

  //------------------------------------------------------------------------
  void CheckAnalysis::finishedTask()
  {
    ++m_finishedTasks;
    auto progressValue = m_finishedTasks * 100 / m_taskList.size();
    reportProgress(progressValue);

    if(m_taskList.size() - m_finishedTasks == 0)
    {
      if(!m_issues.empty())
      {
        emit issuesFound(m_issues);
      }

      resume();
    }
  }

  //------------------------------------------------------------------------
  void CheckAnalysis::addIssue(Issue issue)
  {
    m_issues << issue;
  }

  //------------------------------------------------------------------------
  void CheckTask::checkVolumeIsEmpty() const
  {
    switch(m_item->type())
    {
      case ItemAdapter::Type::SEGMENTATION:
        {
          auto seg = std::dynamic_pointer_cast<SegmentationAdapter>(m_item);
          if(hasVolumetricData(seg->output()))
          {
            auto volume = volumetricData(seg->output());
            if(volume == nullptr || volume->isEmpty())
            {
              Issue segIssue{seg->data().toString(), Severity::CRITICAL, tr("Segmentation has a volume data associated but is empty or null."), tr("Delete segmentation.")};
              emit issue(segIssue);
            }
          }
        }
        break;
      case ItemAdapter::Type::CHANNEL:
        {
          auto channel = std::dynamic_pointer_cast<ChannelAdapter>(m_item);
          if(hasVolumetricData(channel->output()))
          {
            auto volume = volumetricData(channel->output());
            if(volume == nullptr)
            {
              Issue channelIssue{channel->data().toString(), Severity::CRITICAL, tr("Channel has a volume data associated but is null."), tr("Delete channel.")};
              emit issue(channelIssue);
            }
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

    if (hasMeshData(seg->output()))
    {
      auto mesh = meshData(seg->output());

      if(mesh == nullptr || mesh->mesh() == nullptr || mesh->mesh()->GetNumberOfPoints() == 0)
      {
        Issue segIssue{seg->data().toString(), Severity::CRITICAL, tr("Segmentation has a mesh data associated but is empty or null."), tr("Delete segmentation")};
        emit issue(segIssue);
      }
    }
  }

  //------------------------------------------------------------------------
  void CheckTask::checkSegmentationHasChannel() const
  {
    auto seg = std::dynamic_pointer_cast<SegmentationAdapter>(m_item);
    auto channels = QueryAdapter::channels(seg);

    if(channels.empty())
    {
      Issue segIssue{seg->data().toString(), Severity::CRITICAL, tr("Segmentation is not related to any channel."), tr("Delete segmentation.")};
      emit issue(segIssue);
    }
  }

  //------------------------------------------------------------------------
  void CheckTask::checkSegmentationRelations() const
  {
    auto seg = std::dynamic_pointer_cast<SegmentationAdapter>(m_item);
    auto relations = m_model->relations(seg.get(), RelationType::RELATION_INOUT, Sample::CONTAINS);

    if(relations.empty())
    {
      Issue segIssue{seg->data().toString(), Severity::CRITICAL, tr("Segmentation is not related to any sample."), tr("Delete segmentation.")};
      emit issue(segIssue);
    }
  }

  //------------------------------------------------------------------------
  void CheckTask::checkChannelRelations() const
  {
    auto channel = std::dynamic_pointer_cast<ChannelAdapter>(m_item);
    auto relations = m_model->relations(channel.get(), RelationType::RELATION_INOUT, Channel::STAIN_LINK);

    if(relations.empty())
    {
      Issue segIssue{channel->data().toString(), Severity::CRITICAL, tr("Channel is not related to any sample."), tr("Change relations in the \"Channel Explorer\" dialog.")};
      emit issue(segIssue);
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
      Issue segIssue { viewItem->data().toString(), Severity::CRITICAL, tr("Item does not have an output."), tr("Delete item.") };
      emit issue(segIssue);
    }
    else
    {
      int numberOfDatas = 0;
      if (hasMeshData(output))
      {
        checkMeshIsEmpty();
        ++numberOfDatas;
      }

      if (hasVolumetricData(output))
      {
        checkVolumeIsEmpty();
        ++numberOfDatas;
      }

      if (numberOfDatas == 0)
      {
        Issue segIssue { viewItem->data().toString(), Severity::CRITICAL, tr("Item does not have any data."), tr("Delete item.") };
        emit issue(segIssue);
      }
    }

    if (filter == nullptr)
    {
      Issue segIssue { viewItem->data().toString(), Severity::CRITICAL, tr("Item does not have a filter."), tr("Delete item.")};
      emit issue(segIssue);
    }
  }

  //------------------------------------------------------------------------
  void CheckSegmentationTask::run()
  {
    checkViewItemOutputs();
    checkSegmentationHasChannel();
    checkSegmentationRelations();
  }

  //------------------------------------------------------------------------
  void CheckChannelTask::run()
  {
    checkViewItemOutputs();
    checkChannelRelations();
  }

  //------------------------------------------------------------------------
  void CheckSampleTask::run()
  {
    // this page is intentionally left blank :-)
  }


} // namespace ESPINA
