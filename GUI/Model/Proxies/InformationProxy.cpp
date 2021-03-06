/*
 *
 *    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 *    This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

// ESPINA
#include "InformationProxy.h"
#include <GUI/Model/Utils/SegmentationUtils.h>

// Qt
#include <QThread>

using namespace ESPINA;
using namespace ESPINA::Core;
using namespace ESPINA::GUI::Model::Utils;

//------------------------------------------------------------------------
InformationProxy::InformationProxy(SchedulerSPtr scheduler)
: QAbstractProxyModel{}
, m_scheduler        {scheduler}
, m_filter           {nullptr}
{
}

//------------------------------------------------------------------------
InformationProxy::~InformationProxy()
{
  abortTasks();
}

//------------------------------------------------------------------------
void InformationProxy::setSourceModel(ModelAdapterSPtr sourceModel)
{
  if (m_model)
  {
    disconnect(m_model.get(), SIGNAL(rowsInserted(const QModelIndex&, int, int)),
               this, SLOT(sourceRowsInserted(const QModelIndex&, int, int)));
    disconnect(m_model.get(), SIGNAL(rowsAboutToBeRemoved(const QModelIndex&, int, int)),
               this, SLOT(sourceRowsAboutToBeRemoved(QModelIndex, int, int)));
    disconnect(m_model.get(), SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)),
               this, SLOT(sourceDataChanged(const QModelIndex &,const QModelIndex &)));
  }

  beginResetModel();
  m_model = sourceModel;
  m_elements.clear();

  auto tasks = m_pendingInformation.values();

  std::for_each(tasks.begin(), tasks.end(), [](InformationFetcherSPtr &task) { if(!task->hasFinished()) task->abort(); });

  m_pendingInformation.clear();

  if (m_model)
  {
    connect(m_model.get(), SIGNAL(rowsInserted(const QModelIndex&, int, int)),
            this, SLOT(sourceRowsInserted(const QModelIndex&, int, int)));
    connect(m_model.get(), SIGNAL(rowsAboutToBeRemoved(const QModelIndex&, int, int)),
            this, SLOT(sourceRowsAboutToBeRemoved(QModelIndex, int, int)));
    connect(m_model.get(), SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)),
            this, SLOT(sourceDataChanged(const QModelIndex &,const QModelIndex &)));
    connect(m_model.get(), SIGNAL(modelAboutToBeReset()),
            this, SLOT(sourceModelReset()));

    sourceRowsInserted(m_model->segmentationRoot(), 0, m_model->segmentations().size()-1);
  }

  QAbstractProxyModel::setSourceModel(m_model.get());
  endResetModel();
}

//------------------------------------------------------------------------
QModelIndex InformationProxy::mapFromSource(const QModelIndex& sourceIndex) const
{
  if (!sourceIndex.isValid()) return QModelIndex();

  if ( sourceIndex == m_model->classificationRoot()
    || sourceIndex == m_model->sampleRoot()
    || sourceIndex == m_model->channelRoot()
    || sourceIndex == m_model->segmentationRoot())
    return QModelIndex();

  auto sourceItem = itemAdapter(sourceIndex);

  QModelIndex proxyIndex;

  if (isSegmentation(sourceItem) && acceptSegmentation(segmentationPtr(sourceItem)))
  {
    proxyIndex = createIndex(m_elements.indexOf(sourceItem), sourceIndex.column(), sourceItem);
  }

  return proxyIndex;
}

//------------------------------------------------------------------------
QModelIndex InformationProxy::mapToSource(const QModelIndex& proxyIndex) const
{
  if (!proxyIndex.isValid()) return QModelIndex();

  auto proxyItem = itemAdapter(proxyIndex);

  QModelIndex sourceIndex;

  if (isSegmentation(proxyItem))
  {
    sourceIndex = m_model->index(proxyItem);
  }

  return sourceIndex;
}

//------------------------------------------------------------------------
int InformationProxy::columnCount(const QModelIndex& parent) const
{
  if (!parent.isValid())
  {
    return m_keys.size();
  }

  return 0;
}

//------------------------------------------------------------------------
int InformationProxy::rowCount(const QModelIndex& parent) const
{
  if (m_keys.isEmpty()) return 0;

  return !parent.isValid()?m_elements.size():0; //There are no sub-segmentations
}

//------------------------------------------------------------------------
QModelIndex InformationProxy::parent(const QModelIndex& child) const
{
  if (!child.isValid()) return QModelIndex();

  auto childItem = itemAdapter(child);
  if(!childItem || !isSegmentation(childItem)) return QModelIndex();

  return mapFromSource(m_model->segmentationRoot());
}

//------------------------------------------------------------------------
QModelIndex InformationProxy::index(int row, int column, const QModelIndex& parent) const
{
  if (!hasIndex(row, column, parent))
    return QModelIndex();

  if (!parent.isValid())
  {
    // We need to forward all column indices to the same
    // original index while keeping column correct
    return createIndex(row, column, m_elements[row]);
  }

  return QModelIndex();
}

//------------------------------------------------------------------------
QVariant InformationProxy::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (!m_keys.isEmpty() && Qt::DisplayRole == role && section < m_keys.size())
  {
    return m_keys.at(section).value();
  }

  return QAbstractProxyModel::headerData(section, orientation, role);
}

//------------------------------------------------------------------------
QVariant InformationProxy::data(const QModelIndex& proxyIndex, int role) const
{
  if (!proxyIndex.isValid()) return QVariant();

  auto proxyItem = itemAdapter(proxyIndex);
  if (!isSegmentation(proxyItem)) return QVariant();

  auto segmentation = segmentationPtr(proxyItem);

  if(!acceptSegmentation(segmentation)) return QVariant();

  if (role == Qt::TextAlignmentRole) return Qt::AlignRight;

  if (role == Qt::UserRole && proxyIndex.column() == 0)
  {
    const int HIDE_PROGRESS = -1;
    int progress = HIDE_PROGRESS;

    if (m_pendingInformation.contains(segmentation))
    {
      auto task = m_pendingInformation[segmentation];

      progress = task->hasFinished()?HIDE_PROGRESS:task->currentProgress();
    }

    return progress;
  }

  if(role == Qt::ForegroundRole || role == Qt::BackgroundRole)
  {
    QVariant::fromValue(Qt::black);
    if(proxyIndex.column() == 0) return QAbstractProxyModel::data(proxyIndex, role);

    if (!m_pendingInformation.contains(segmentation) || !m_pendingInformation[segmentation]->hasFinished())
    {
      return role == Qt::ForegroundRole ? QVariant::fromValue(Qt::black) : QVariant::fromValue(Qt::lightGray);
    }

    auto info = data(proxyIndex, Qt::DisplayRole);
    if(info.canConvert(QVariant::String) && (info.toString().contains("Fail", Qt::CaseInsensitive) || info.toString().contains("Error", Qt::CaseInsensitive)))
    {
      return role == Qt::ForegroundRole ? QVariant::fromValue(Qt::white) : QVariant::fromValue(Qt::red);
    }

    return QAbstractProxyModel::data(proxyIndex, role);
  }

  if (role == Qt::DisplayRole && !m_keys.isEmpty())
  {
    auto key = m_keys[proxyIndex.column()];

    if (NameKey() == key)
    {
      return segmentation->data(role);
    }

    if (CategoryKey() == key)
    {
      return segmentation->category()->data(role);
    }

    const auto extensions = segmentation->readOnlyExtensions();

    if (extensions->hasInformation(key))
    {
      if (!m_pendingInformation.contains(segmentation) || m_pendingInformation[segmentation]->isAborted())
      {
        auto task = std::make_shared<InformationFetcher>(segmentation, m_keys, m_scheduler);
        m_pendingInformation[segmentation] = task;

        if (!task->hasFinished()) // If all information is available on constructor, it is set as finished
        {
          connect(task.get(), SIGNAL(progress(int)),
                  this,       SLOT(onProgressReported(int)));
          connect(task.get(), SIGNAL(finished()),
                  this,       SLOT(onTaskFininished()));
          Task::submit(task);
        }
        else // we avoid overloading the scheduler
        {
          return extensions->information(key);
        }
      }
      else if (m_pendingInformation[segmentation]->hasFinished())
      {
        auto info = segmentation->information(key);
        if (!info.isValid())
        {
          info = tr("Unavailable");
        }
        return info;
      }

      return QString();

    }
    else
    {
      return tr("Unavailable");
    }
  }
  else if (proxyIndex.column() > 0)
  {
    return QVariant();//To avoid checkrole or other roles
  }

  return QAbstractProxyModel::data(proxyIndex, role);
}

//------------------------------------------------------------------------
void InformationProxy::setCategory(const QString &classificationName)
{
  beginResetModel();
  m_category = classificationName;
  endResetModel();
}

//------------------------------------------------------------------------
void InformationProxy::setFilter(const SegmentationAdapterList *filter)
{
  beginResetModel();
  m_filter = filter;
  endResetModel();
}

//------------------------------------------------------------------------
void InformationProxy::setInformationTags(const SegmentationExtension::InformationKeyList &keys)
{
  beginResetModel();

  abortTasks();

  m_keys = keys;

  endResetModel();
}

//------------------------------------------------------------------------
int InformationProxy::progress() const
{
  const auto tasks = m_pendingInformation.values();

  double finishedTasks = std::count_if(tasks.constBegin(), tasks.constEnd(), [](const InformationFetcherSPtr &task){ return task->hasFinished(); });

  return finishedTasks / rowCount() * 100;
}

//------------------------------------------------------------------------
void InformationProxy::sourceRowsInserted(const QModelIndex& sourceParent, int start, int end)
// Avoid population the view if no query is selected
{
  if (sourceParent == m_model->segmentationRoot())
  {
    ItemAdapterList acceptedItems;

    // Filter items
    for (int row = start; row <= end; row++)
    {
      auto item = itemAdapter(sourceParent.child(row, 0));

      auto segmentation = segmentationPtr(item);
      if (acceptSegmentation(segmentation))
      {
        acceptedItems << item;
      }
    }

    int startRow = m_elements.size();
    int endRow   = startRow + acceptedItems.size() - 1;
    // Avoid populating the view if no query is selected
    if (!m_keys.isEmpty())
    {
      beginInsertRows(QModelIndex(), startRow, endRow);
    }

    for(auto acceptedItem : acceptedItems)
    {
      m_elements << acceptedItem;
    }

    if (!m_keys.isEmpty())
    {
      endInsertRows();
    }
  }
}

//------------------------------------------------------------------------
void InformationProxy::sourceRowsAboutToBeRemoved(const QModelIndex& sourceParent, int start, int end)
{
  if (sourceParent == m_model->segmentationRoot())
  {
    QModelIndexList removedIndexes;
    for (int row = start; row <= end; row++)
    {
      QModelIndex proxyIndex = mapFromSource(sourceParent.child(row, 0));
      if (proxyIndex.isValid())
        removedIndexes << proxyIndex;
    }

    if (!removedIndexes.isEmpty())
    {
      // Avoid population the view if no query is selected
      if (!m_keys.isEmpty())
        beginRemoveRows(QModelIndex(), removedIndexes.first().row(), removedIndexes.last().row());
      for(auto index : removedIndexes)
      {
        // We use start instead of row to avoid access to removed indices
        auto removedItem = itemAdapter(index);
        m_elements.removeOne(removedItem);
      }
      if (!m_keys.isEmpty())
        endRemoveRows();
    }
  }
}

//------------------------------------------------------------------------
void InformationProxy::sourceDataChanged(const QModelIndex& sourceTopLeft, const QModelIndex& sourceBottomRight)
{
  if (sourceTopLeft.parent() == m_model->segmentationRoot() && (sourceTopLeft == sourceBottomRight))
  {
    auto item = itemAdapter(sourceTopLeft);
    auto segmentation = segmentationPtr(item);

    if (m_elements.contains(item) && !acceptSegmentation(segmentation))
    {
      int row = m_elements.indexOf(item);

      beginRemoveRows(QModelIndex(), row, row);
      m_elements.removeAt(row);
      endRemoveRows();
    }
    else if (!m_elements.contains(item) && acceptSegmentation(segmentation))
    {
      int row = m_elements.size();

      beginInsertRows(QModelIndex(), row, row);
      m_elements << item;
      endInsertRows();

    }
    else
    {
      emit dataChanged(mapFromSource(sourceTopLeft), mapFromSource(sourceBottomRight));
    }
  }
}


//------------------------------------------------------------------------
void InformationProxy::onProgressReported(int progress)
{
  emit informationProgress();
}

//------------------------------------------------------------------------
void InformationProxy::onTaskFininished()
{
  emit informationProgress();
}

//------------------------------------------------------------------------
void InformationProxy::sourceModelReset()
{
  beginResetModel();
  {
    m_elements.clear();
  }
  endResetModel();
}

//------------------------------------------------------------------------
bool InformationProxy::acceptSegmentation(const SegmentationAdapterPtr segmentation) const
{
  return segmentation->category()->classificationName() == m_category
      && (m_filter->isEmpty() || m_filter->contains(segmentation));
}

//------------------------------------------------------------------------
QModelIndex InformationProxy::index(const ItemAdapterPtr segmentation, int col)
{
  int row = m_elements.indexOf(segmentation);

  return index(row, col);
}

//------------------------------------------------------------------------
void InformationProxy::abortTasks()
{
  for (auto task : m_pendingInformation)
  {
    disconnect(task.get(), SIGNAL(progress(int)),
               this,       SLOT(onProgressReported(int)));

    disconnect(task.get(), SIGNAL(finished()),
               this,       SLOT(onTaskFininished()));

    if (!task->hasFinished())
    {
      task->abort();

      if(!task->thread()->wait(100))
      {
        task->thread()->terminate();
      }
    }
  }

  m_pendingInformation.clear();
}
