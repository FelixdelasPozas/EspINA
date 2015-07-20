/*
 *
 * Copyright (C) 2014 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>
 *
 * This file is part of ESPINA.
 *
 *    ESPINA is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

// ESPINA
#include <Core/Analysis/Data/VolumetricData.hxx>
#include <Core/Analysis/Data/MeshData.h>
#include <Core/Analysis/Data/SkeletonData.h>
#include <Core/Analysis/Sample.h>
#include <Core/Analysis/Channel.h>
#include <Dialogs/IssueList/CheckAnalysis.h>
#include <GUI/Model/Utils/QueryAdapter.h>

// Qt
#include <QDebug>

using namespace ESPINA;

//------------------------------------------------------------------------
CheckAnalysis::CheckAnalysis(SchedulerSPtr scheduler, ModelAdapterSPtr model)
: Task           {scheduler}
, m_finishedTasks{0}
{
  setDescription(tr("Issues checker"));
  setPriority(Priority::LOW);

  qRegisterMetaType<Issue>("Issue");
  qRegisterMetaType<IssueList>("IssueList");

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

  m_taskList << std::make_shared<CheckDuplicatedSegmentationsTask>(scheduler, model);
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
  QMutexLocker lock(&m_progressMutex);

  ++m_finishedTasks;

  auto tasksNum = m_taskList.size();
  auto progressValue = m_finishedTasks * 100 / tasksNum;
  reportProgress(progressValue);

  if(tasksNum - m_finishedTasks == 0)
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
  QMutexLocker lock(&m_progressMutex);

  m_issues << issue;
}

//------------------------------------------------------------------------
void CheckDataTask::checkViewItemOutputs(ViewItemAdapterSPtr viewItem) const
{
  auto output = viewItem->output();
  auto filter = viewItem->filter();

  if (output == nullptr)
  {
    qWarning() << "ViewItem" << viewItem->data().toString() << "doesn't have output.";

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

    if (hasSkeletonData(output))
    {
      checkSkeletonIsEmpty();
      ++numberOfDatas;
    }

    if (numberOfDatas == 0)
    {
      qWarning() << "ViewItem" << viewItem->data().toString() << "doesn't have data.";

      Issue segIssue { viewItem->data().toString(), Severity::CRITICAL, tr("Item does not have any data at all."), tr("Delete item.") };
      emit issue(segIssue);
    }
  }

  if (filter == nullptr)
  {
    qWarning() << "ViewItem" << viewItem->data().toString() << "doesn't have filter.";

    Issue segIssue { viewItem->data().toString(), Severity::CRITICAL, tr("Can't find the origin of the item."), tr("Delete item.")};
    emit issue(segIssue);
  }
}

//------------------------------------------------------------------------
CheckSegmentationTask::CheckSegmentationTask(SchedulerSPtr scheduler, NeuroItemAdapterSPtr item, ModelAdapterSPtr model)
: CheckDataTask{scheduler, item, model}
, m_segmentation   {std::dynamic_pointer_cast<SegmentationAdapter>(item)}
{
}

//------------------------------------------------------------------------
void CheckSegmentationTask::run()
{
  checkViewItemOutputs(m_segmentation);
  checkHasChannel();
  checkRelations();
}

//------------------------------------------------------------------------
void CheckSegmentationTask::checkVolumeIsEmpty() const
{
  auto volume = readLockVolume(m_segmentation->output());
  if (volume.isNull() || volume->isEmpty())
  {
    if (volume.isNull())
    {
      qWarning() << tr("ViewItem %1 problem. Volume is null.").arg(m_segmentation->data().toString());
    }

    Issue segIssue{ m_segmentation->data().toString(), Severity::CRITICAL, tr("Segmentation has a volume associated but is empty."), tr("Delete segmentation.") };
    emit issue(segIssue);
  }
}

//------------------------------------------------------------------------
void CheckSegmentationTask::checkMeshIsEmpty() const
{
  auto mesh = readLockMesh(m_segmentation->output());

  if (mesh.isNull() || mesh->mesh() == nullptr || mesh->mesh()->GetNumberOfPoints() == 0)
  {
    if (mesh.isNull() || mesh->mesh() == nullptr)
    {
      qWarning() << tr("ViewItem %1 problem. Mesh is null or redirects to null.").arg(m_segmentation->data().toString());
    }

    Issue segIssue{ m_segmentation->data().toString(), Severity::CRITICAL, tr("Segmentation has a mesh associated but is empty."), tr("Delete segmentation") };

    emit issue(segIssue);
  }
}

//------------------------------------------------------------------------
void CheckSegmentationTask::checkSkeletonIsEmpty() const
{
  auto skeleton = readLockSkeleton(m_segmentation->output());

  if (skeleton.isNull() || skeleton->skeleton() == nullptr || skeleton->skeleton()->GetNumberOfPoints() == 0)
  {
    if (skeleton.isNull() || skeleton->skeleton() == nullptr)
    {
      qWarning() << tr("ViewItem %1 problem. Skeleton is null or redirects to null.").arg(m_segmentation->data().toString());
    }

    Issue segIssue{ m_segmentation->data().toString(), Severity::CRITICAL, tr("Segmentation has a skeleton associated but is empty."), tr("Delete segmentation") };
    emit issue(segIssue);
  }
}

//------------------------------------------------------------------------
void CheckSegmentationTask::checkHasChannel() const
{
  auto channels = QueryAdapter::channels(m_segmentation);

  if(channels.empty())
  {
    Issue segIssue{m_segmentation->data().toString(), Severity::CRITICAL, tr("Segmentation is not related to any channel."), tr("Delete segmentation.")};
    emit issue(segIssue);
  }
}

//------------------------------------------------------------------------
void CheckSegmentationTask::checkRelations() const
{
  auto relations = m_model->relations(m_segmentation.get(), RelationType::RELATION_INOUT, Sample::CONTAINS);

  if(relations.empty())
  {
    Issue segIssue{m_segmentation->data().toString(), Severity::CRITICAL, tr("Segmentation is not related to any sample."), tr("Delete segmentation.")};
    emit issue(segIssue);
  }
}

//------------------------------------------------------------------------
CheckChannelTask::CheckChannelTask(SchedulerSPtr scheduler, NeuroItemAdapterSPtr item, ModelAdapterSPtr model)
: CheckDataTask{scheduler, item, model}
, m_channel   {std::dynamic_pointer_cast<ChannelAdapter>(item)}
{
}

//------------------------------------------------------------------------
void CheckChannelTask::checkVolumeIsEmpty() const
{
  if(hasVolumetricData(m_channel->output()))
  {
    auto volume = readLockVolume(m_channel->output());
    if(volume.isNull())
    {
      qWarning() << tr("ViewItem %1 problem. Volume is null.").arg(m_channel->data().toString());

      Issue channelIssue{m_channel->data().toString(), Severity::CRITICAL, tr("Channel has a volume associated but can't find it."), tr("Delete channel.")};
      emit issue(channelIssue);
    }
  }
}

//------------------------------------------------------------------------
void CheckChannelTask::checkRelations() const
{
  auto relations = m_model->relations(m_channel.get(), RelationType::RELATION_INOUT, Channel::STAIN_LINK);

  if(relations.empty())
  {
    Issue segIssue{m_channel->data().toString(), Severity::CRITICAL, tr("Channel is not related to any sample."), tr("Change relations in the \"Channel Explorer\" dialog.")};
    emit issue(segIssue);
  }
}

//------------------------------------------------------------------------
void CheckChannelTask::run()
{
  checkViewItemOutputs(m_channel);
  checkRelations();
}

//------------------------------------------------------------------------
CheckSampleTask::CheckSampleTask(SchedulerSPtr scheduler, NeuroItemAdapterSPtr item, ModelAdapterSPtr model)
: CheckDataTask{scheduler, item, model}
, m_sample   {std::dynamic_pointer_cast<SampleAdapter>(item)}
{
}

//------------------------------------------------------------------------
void CheckSampleTask::run()
{
  // this page is intentionally left blank :-)
}

//------------------------------------------------------------------------
CheckDuplicatedSegmentationsTask::CheckDuplicatedSegmentationsTask(SchedulerSPtr scheduler, ModelAdapterSPtr model)
: CheckTask(scheduler, model)
{
}

//------------------------------------------------------------------------
void CheckDuplicatedSegmentationsTask::run()
{
  auto segmentations = m_model->segmentations();

  for (int i = 0; i < segmentations.size(); ++i)
  {
    auto seg_i    = segmentations[i].get();
    auto bounds_i = seg_i->bounds();

    for (int j = i + 1; j < segmentations.size(); ++j)
    {
      auto seg_j    = segmentations[j].get();
      auto bounds_j = seg_j->bounds();

      if (seg_i->category() == seg_j->category())
      {
        if (contains(bounds_i, bounds_j))
        {
          emit issue(possibleDuplication(seg_i, seg_j));
        }
        else if (contains(bounds_j, bounds_i))
        {
          emit issue(possibleDuplication(seg_j, seg_i));
        }
      }
    }
  }
}

//------------------------------------------------------------------------
Issue CheckDuplicatedSegmentationsTask::possibleDuplication(SegmentationAdapterPtr seg1, SegmentationAdapterPtr seg2) const
{
  auto title   = seg2->data().toString();
  auto message = tr("Possible duplicated segmentation of %1").arg(seg1->data().toString());
  auto hint    = tr("Remove unnecesary segmentation");
  return Issue{title, Severity::WARNING, message, hint};
}

