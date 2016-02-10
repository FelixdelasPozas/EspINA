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
#include <Core/Analysis/Data/VolumetricDataUtils.hxx>
#include <Core/Utils/EspinaException.h>
#include <Dialogs/IssueList/CheckAnalysis.h>
#include <GUI/Model/Utils/QueryAdapter.h>
#include <GUI/Model/Utils/SegmentationUtils.h>
#include <Extensions/Issues/SegmentationIssues.h>

using namespace ESPINA;
using namespace ESPINA::Core::Utils;
using namespace ESPINA::Extensions;
using namespace ESPINA::GUI::Model::Utils;

const QString DUPLICATED_TAG = "duplicated";

//------------------------------------------------------------------------
class NeuroItemIssue
: public Issue
{
  public:
    explicit NeuroItemIssue(NeuroItemAdapterPtr item, const Severity severity, const QString& description, const QString& suggestion = QString())
    : Issue(item->data(Qt::DisplayRole).toString(), severity, description, suggestion)
    {}
};

//------------------------------------------------------------------------
class SegmentationIssue
: public NeuroItemIssue
{
  public:
    explicit SegmentationIssue(SegmentationAdapterPtr segmentation, const Severity severity, const QString& description, const QString& suggestion = QString())
    : NeuroItemIssue(segmentation, severity, description, suggestion)
    {
      addSeverityTag(segmentation, severity);
    }

  protected:
    void addIssueTag(SegmentationAdapterPtr segmentation, const QStringList tags) const
    {
      auto tagExtensions = retrieveOrCreateExtension<SegmentationTags>(segmentation->extensions());

      for (auto tag : tags)
      {
        tagExtensions->addTag(tag);
      }
    }

  private:
    void addSeverityTag(SegmentationAdapterPtr segmentation, const Severity severity)
    {
      switch(severity)
      {
        case Severity::CRITICAL:
          addIssueTag(segmentation, { Issue::CRITICAL_TAG });
          break;
        case Severity::WARNING:
          addIssueTag(segmentation, { Issue::WARNING_TAG });
          break;
        case Severity::INFORMATION:
          addIssueTag(segmentation, { Issue::INFORMATION_TAG });
          break;
        default:
          break;
      }
    }
};

//------------------------------------------------------------------------
class DuplicatedIssue
: public SegmentationIssue
{
  public:
    explicit DuplicatedIssue(SegmentationAdapterPtr original, SegmentationAdapterPtr duplicated, const unsigned long long duplicatedVoxels)
    : SegmentationIssue(original, Severity::WARNING, description(duplicated, duplicatedVoxels), suggestion())
    {
      addIssueTag(original, { DUPLICATED_TAG });
    }

  private:
    static QString description(const SegmentationAdapterPtr segmentation, const unsigned long long duplicatedVoxels)
    {
      return QObject::tr("Possible duplicated segmentation of %1. Both have %2 voxel%3 in common.").arg(segmentation->data(Qt::DisplayRole).toString())
                                                                                                   .arg(duplicatedVoxels)
                                                                                                   .arg((duplicatedVoxels > 1) ? "s" : "");
    }

    static QString suggestion()
    {
      return QObject::tr("Delete one or edit both segmentations to avoid common voxels.");
    }

};

//------------------------------------------------------------------------
void CheckTask::reportIssue(NeuroItemAdapterSPtr item,
                            const Issue::Severity& severity,
                            const QString& description,
                            const QString& suggestion) const
{
  IssueSPtr issue;

  if (isSegmentation(item.get()))
  {
    auto segmentation = segmentationPtr(item.get());

    issue = std::make_shared<SegmentationIssue>(segmentation, severity, description, suggestion);
  }
  else
  {
   issue = std::make_shared<NeuroItemIssue>(item.get(), severity, description, suggestion);
  }

  reportIssue(item.get(), issue);
}

//------------------------------------------------------------------------
void CheckTask::reportIssue(NeuroItemAdapterPtr item, IssueSPtr issue) const
{
  if (item && isSegmentation(item))
  {
    auto segmentation   = segmentationPtr(item);
    auto issueExtension = retrieveOrCreateExtension<SegmentationIssues>(segmentation->extensions());
    issueExtension->addIssue(issue);
  }

  emit issueFound(issue);
}

//------------------------------------------------------------------------
QString CheckTask::deleteHint(NeuroItemAdapterSPtr item) const
{
  return tr("Delete %1").arg(isSegmentation(item.get()) ? "segmentation" : "channel");
}

//------------------------------------------------------------------------
CheckAnalysis::CheckAnalysis(SchedulerSPtr scheduler, ModelAdapterSPtr model)
: Task{scheduler}
, m_finishedTasks{0}
{
  setDescription(tr("Issues checker"));
  setPriority(Priority::LOW);

  qRegisterMetaType<Extensions::IssueList>("Extensions::IssueList");
  qRegisterMetaType<Extensions::IssueSPtr>("Extensions::IssueSPtr");

  removePreviousIssues(model);

  for(auto seg: model->segmentations())
  {
    m_checkList << std::make_shared<CheckSegmentationTask>(scheduler, seg, model);
  }

  for(auto channel: model->channels())
  {
    m_checkList << std::make_shared<CheckChannelTask>(scheduler, channel, model);
  }

  for(auto sample: model->samples())
  {
    m_checkList << std::make_shared<CheckSampleTask>(scheduler, sample, model);
  }

  m_checkList << std::make_shared<CheckDuplicatedSegmentationsTask>(scheduler, model);
}

//------------------------------------------------------------------------
void CheckAnalysis::run()
{
  for(auto task: m_checkList)
  {
    // needs to be direct connection because the mutexes.
    connect(task.get(), SIGNAL(finished()),
            this,       SLOT(finishedTask()), Qt::DirectConnection);

    connect(task.get(), SIGNAL(issueFound(Extensions::IssueSPtr)),
            this,       SLOT(addIssue(Extensions::IssueSPtr)), Qt::DirectConnection);

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

  auto tasksNum = m_checkList.size();
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
void CheckAnalysis::addIssue(IssueSPtr issue)
{
  QMutexLocker lock(&m_progressMutex);

  m_issues << issue;
}

//------------------------------------------------------------------------
void CheckAnalysis::removePreviousIssues(ModelAdapterSPtr model)
{
  for(auto segmentation: model->segmentations())
  {
    auto extensions = segmentation->extensions();
    if(extensions->hasExtension(SegmentationIssues::TYPE))
    {
      extensions->remove(SegmentationIssues::TYPE);
    }

    if(extensions->hasExtension(SegmentationTags::TYPE))
    {
      auto tagExtension = retrieveExtension<SegmentationTags>(extensions);

      for(auto tag: tagExtension->tags())
      {
        for(auto issueTag: {Issue::CRITICAL_TAG, Issue::INFORMATION_TAG, Issue::WARNING_TAG, DUPLICATED_TAG})
        if(tag.compare(issueTag) == 0)
        {
          tagExtension->removeTag(tag);
        }
      }

      if(tagExtension->tags().isEmpty())
      {
        extensions->remove(tagExtension);
      }
    }
  }

  for(auto channel: model->channels())
  {
    // TODO: implement ChannelIssues channel extension
  }

  for(auto sample: model->samples())
  {
    // EMPTY intentionally empty
  }
}

//------------------------------------------------------------------------
void CheckDataTask::checkViewItemOutputs(ViewItemAdapterSPtr viewItem) const
{
  auto output = viewItem->output();
  auto filter = viewItem->filter();

  if (output == nullptr)
  {
    auto description = tr("Item does not have an output.");

    reportIssue(viewItem, Issue::Severity::CRITICAL, description, deleteHint(viewItem));
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

//     if (hasSkeletonData(output))
//     {
//       checkSkeletonIsEmpty();
//       ++numberOfDatas;
//     }

    if (numberOfDatas == 0)
    {
      auto description = tr("Item does not have any data at all");

      reportIssue(viewItem, Issue::Severity::CRITICAL, description, deleteHint(viewItem));
    }


    auto spacing = output->spacing();
    if (spacing[0] == 0 || spacing[1] == 0 || spacing[2] == 0)
    {
      auto description = tr("Invalid output spacing");

      reportIssue(viewItem, Issue::Severity::CRITICAL, description, deleteHint(viewItem));
    }
  }

  if (filter == nullptr)
  {
    auto description = tr("Can't find the origin of the item");

    reportIssue(viewItem, Issue::Severity::CRITICAL, description, deleteHint(viewItem));
  }
}

//------------------------------------------------------------------------
CheckSegmentationTask::CheckSegmentationTask(SchedulerSPtr scheduler,
                                             NeuroItemAdapterSPtr item,
                                             ModelAdapterSPtr model)
: CheckDataTask{scheduler, item, model}
, m_segmentation{std::dynamic_pointer_cast<SegmentationAdapter>(item)}
{
}

//------------------------------------------------------------------------
void CheckSegmentationTask::run()
{
  try
  {
    checkViewItemOutputs(m_segmentation);
    checkHasChannel();
    checkRelations();
  }
  catch(const EspinaException &e)
  {
    reportIssue(m_segmentation, Issue::Severity::CRITICAL, tr("Check crashed during testing."), e.details());
  }
}

//------------------------------------------------------------------------
void CheckSegmentationTask::checkVolumeIsEmpty() const
{
  auto volume = readLockVolume(m_segmentation->output());
  if (volume->isEmpty())
  {
    auto description = tr("Segmentation has a volume associated but is empty");

    reportIssue(m_segmentation, Issue::Severity::CRITICAL, description, deleteHint(m_item));
  }
  else
  {
    checkDataBounds(volume);
  }
}

//------------------------------------------------------------------------
void CheckSegmentationTask::checkMeshIsEmpty() const
{
  auto mesh = readLockMesh(m_segmentation->output());

  if (mesh->mesh() == nullptr || mesh->mesh()->GetNumberOfPoints() == 0)
  {
    auto description = tr("Segmentation has a mesh associated but is empty");

    reportIssue(m_segmentation, Issue::Severity::CRITICAL, description, deleteHint(m_item));
  }
  else
  {
    checkDataBounds(mesh);
  }
}

//------------------------------------------------------------------------
void CheckSegmentationTask::checkSkeletonIsEmpty() const
{
//   auto skeleton = readLockSkeleton(m_segmentation->output());
//
//   if (skeleton.isNull() || skeleton->skeleton() == nullptr || skeleton->skeleton()->GetNumberOfPoints() == 0)
//   {
//     auto description = tr("Segmentation has a skeleton associated but is empty");
//
//     reportIssue(m_segmentation, Issue::Severity::CRITICAL, description, deleteHint(m_item));
//   }
}

//------------------------------------------------------------------------
void CheckSegmentationTask::checkRelations() const
{
  auto relations = m_model->relations(m_segmentation.get(), RelationType::RELATION_INOUT, Sample::CONTAINS);

  if(relations.empty())
  {
    auto description = tr("Segmentation is not related to any sample");

    reportIssue(m_segmentation, Issue::Severity::CRITICAL, description, deleteHint(m_item));
  }
}

//------------------------------------------------------------------------
void CheckSegmentationTask::checkHasChannel() const
{
  auto channels = QueryAdapter::channels(m_segmentation);

  if(channels.empty())
  {
    auto description = tr("Segmentation is not related to any stack");

    reportIssue(m_segmentation, Issue::Severity::CRITICAL, description, deleteHint(m_item));
  }
  else
  {
    if (channels.size() > 1)
    {
      auto description = tr("Segmentation is related to several stacks");

      reportIssue(m_segmentation, Issue::Severity::WARNING, description, deleteHint(m_item));
    }
    else
    {
      checkIsInsideChannel(channels.first().get());
    }
  }
}

//------------------------------------------------------------------------
void CheckSegmentationTask::checkIsInsideChannel(ChannelAdapterPtr channel) const
{
  if (!contains(channel->output()->bounds(), m_segmentation->output()->bounds(), channel->output()->spacing()))
  {
    auto description = tr("Segmentation is partially outside its stack");

    reportIssue(m_segmentation, Issue::Severity::WARNING, description, deleteHint(m_item));
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
    if(volume->isEmpty())
    {
      auto description = tr("Stack has a volume associated but it's empty");

      reportIssue(m_channel, Issue::Severity::CRITICAL, description, deleteHint(m_item));
    }
  }
}

//------------------------------------------------------------------------
void CheckChannelTask::checkRelations() const
{
  auto relations = m_model->relations(m_channel.get(), RelationType::RELATION_INOUT, Channel::STAIN_LINK);

  if(relations.empty())
  {
    auto description = tr("Stack is not related to any sample");
    auto hint        = tr("Change relations in the \"Stack Explorer\" panel");

    reportIssue(m_channel, Issue::Severity::CRITICAL, description, hint);
  }
}

//------------------------------------------------------------------------
void CheckChannelTask::run()
{
  try
  {
    checkViewItemOutputs(m_channel);
    checkRelations();
  }
  catch(const EspinaException &e)
  {
    reportIssue(m_channel, Issue::Severity::CRITICAL, tr("Check crashed during testing."), e.details());
  }
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
: CheckTask      {scheduler, model}
, m_segmentations{model->segmentations()}
{
  // detach from implicit sharing
  m_segmentations.detach();
}

//------------------------------------------------------------------------
void CheckDuplicatedSegmentationsTask::run()
{
  for (int i = 0; i < m_segmentations.size(); ++i)
  {
    auto seg_i    = m_segmentations[i].get();
    auto bounds_i = readLockVolume(seg_i->output())->bounds();

    for (int j = i + 1; j < m_segmentations.size(); ++j)
    {
      auto seg_j    = m_segmentations[j].get();
      auto bounds_j = readLockVolume(seg_j->output())->bounds();

      if (seg_i->category() == seg_j->category())
      {
        if(intersect(bounds_i, bounds_j))
        {
          auto commonBounds = intersection(bounds_i, bounds_j);
          if(commonBounds.areValid())
          {
            auto image1     = readLockVolume(seg_i->output())->itkImage(commonBounds);
            auto image2     = readLockVolume(seg_j->output())->itkImage(commonBounds);
            auto duplicated = compare_images<itkVolumeType>(image1, image2, commonBounds);

            if(duplicated > 0)
            {
              reportIssue(seg_i, possibleDuplication(seg_i, seg_j, duplicated));
              reportIssue(seg_j, possibleDuplication(seg_j, seg_i, duplicated));
            }
          }
        }
      }
    }
  }
}

//------------------------------------------------------------------------
IssueSPtr CheckDuplicatedSegmentationsTask::possibleDuplication(SegmentationAdapterPtr original,
                                                                SegmentationAdapterPtr duplicated,
                                                                const unsigned long long duplicatedVoxes) const
{
  return std::make_shared<DuplicatedIssue>(original, duplicated, duplicatedVoxes);
}

//------------------------------------------------------------------------
CheckDataTask::CheckDataTask(SchedulerSPtr scheduler, NeuroItemAdapterSPtr item, ModelAdapterSPtr model)
: CheckTask{scheduler, model}
, m_item(item)
{
  setDescription(tr("Checking %1").arg(item->data().toString())); // for debugging, the user will never see this
}

//------------------------------------------------------------------------
template<typename T> void CheckDataTask::checkDataBounds(Output::ReadLockData<T> &data) const
{
  if (!data->bounds().areValid())
  {
    auto description = tr("%1 has invalid bounds.").arg(data->type());

    reportIssue(m_item, Extensions::Issue::Severity::CRITICAL, description, deleteHint(m_item));
  }
}
