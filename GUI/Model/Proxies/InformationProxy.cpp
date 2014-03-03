/*
 *    <one line to give the program's name and a brief idea of what it does.>
 *    Copyright (C) 2012  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 *    This program is free software: you can redistribute it and/or modify
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


#include "InformationProxy.h"

using namespace EspINA;

class InformationProxy::InformationFetcher
: public Task
{
public:
  InformationFetcher(SegmentationAdapterPtr segmentation,
                     const SegmentationExtension::InfoTagList &tags,
                     SchedulerSPtr scheduler)
  : Task(scheduler)
  , Segmentation(segmentation)
  , m_tags(tags)
  , m_progress(0)
  {
    auto id = Segmentation->data(Qt::DisplayRole).toString();
    setDescription(tr("%1 information").arg(id));
    setHidden(true);
  }

  SegmentationAdapterPtr Segmentation;

  int currentProgress() const
  { return m_progress; }

protected:
  virtual void run()
  {
    for (int i = 0; i < m_tags.size(); ++i)
    {
      if (!canExecute()) break;

      auto tag = m_tags[i];
      if (tag != "Name" && tag != "Category")
      {
        Segmentation->information(m_tags[i]);
      }

      m_progress = (100.0*i)/m_tags.size();
      emit progress(m_progress);
    }
  }

private:
  const SegmentationExtension::InfoTagList m_tags;
  int   m_progress;
};

//------------------------------------------------------------------------
InformationProxy::InformationProxy(SchedulerSPtr scheduler)
: QAbstractProxyModel()
, m_scheduler(scheduler)
{
}

//------------------------------------------------------------------------
InformationProxy::~InformationProxy()
{
  for (auto task : m_pendingInformation)
  {
    if (!task->hasFinished())
    {
      task->abort();
    }
  }
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

  m_model = sourceModel;
  m_elements.clear();

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
}

//------------------------------------------------------------------------
QModelIndex InformationProxy::mapFromSource(const QModelIndex& sourceIndex) const
{
  if (!sourceIndex.isValid())
    return QModelIndex();

  if ( sourceIndex == m_model->segmentationRoot()
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
  if (!proxyIndex.isValid())
    return QModelIndex();

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
    return m_tags.size();
  }

  return 0;
}

//------------------------------------------------------------------------
int InformationProxy::rowCount(const QModelIndex& parent) const
{
  if (m_tags.isEmpty())
    return 0;

  if (!parent.isValid())
    return m_elements.size();
  else
    return 0;// There are no sub-segmentations
}

//------------------------------------------------------------------------
QModelIndex InformationProxy::parent(const QModelIndex& child) const
{
  if (!child.isValid())
    return QModelIndex();

  auto childItem = itemAdapter(child);
  Q_ASSERT(isSegmentation(childItem));

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
  if (m_tags.isEmpty())
    return QAbstractProxyModel::headerData(section, orientation, role);

  if (Qt::DisplayRole == role && section < m_tags.size())
  {
    return m_tags[section];
  }

  return QAbstractProxyModel::headerData(section, orientation, role);
}

//------------------------------------------------------------------------
QVariant InformationProxy::data(const QModelIndex& proxyIndex, int role) const
{
  if (!proxyIndex.isValid())
    return QVariant();

  auto proxyItem = itemAdapter(proxyIndex);
  if (!isSegmentation(proxyItem))
    return QVariant();

  auto segmentation = segmentationPtr(proxyItem);

  if (role == Qt::TextAlignmentRole)
  {
    return Qt::AlignRight;
  }

  if (role == Qt::UserRole && proxyIndex.column() == 0)
  {
    const int HIDE_PROGRESS = -1;
    int progress = HIDE_PROGRESS;

    if (m_pendingInformation.contains(segmentation))
    {

      InformationFetcherSPtr task = m_pendingInformation[segmentation];

      progress = task->hasFinished()?HIDE_PROGRESS:task->currentProgress();
    }

    return progress;
  }


  if (role == Qt::DisplayRole && !m_tags.isEmpty())
  {
    auto tag = m_tags[proxyIndex.column()];

    if (tr("Name") == tag)
    {
      return segmentation->data(role);
    }

    if (tr("Category") == tag)
    {
      return segmentation->category()->data(role);
    }

    if (segmentation->informationTags().contains(tag))
    {
      if (!m_pendingInformation.contains(segmentation)
       || m_pendingInformation[segmentation]->isAborted())
      {
        InformationFetcherSPtr task{new InformationFetcher(segmentation, m_tags, m_scheduler)};
        m_pendingInformation[segmentation] = task;
        connect(task.get(), SIGNAL(progress(int)),
                this, SLOT(onProgessReported(int)));
        connect(task.get(), SIGNAL(finished()),
                this, SLOT(onTaskFininished()));
        Task::submit(task);
      } else if (m_pendingInformation[segmentation]->hasFinished())
      {
        return segmentation->information(tag);
      }

      return "Computing";

    } else
    {
      return tr("Unavailable");
    }
  } else if (proxyIndex.column() > 0)
    return QVariant();//To avoid checkrole or other roles

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
void InformationProxy::setInformationTags(const SegmentationExtension::InfoTagList tags)
{
  beginResetModel();
  for (auto task : m_pendingInformation)
  {
    task->abort();
  }
  m_pendingInformation.clear();
  m_tags = tags;
  endResetModel();
}

// //------------------------------------------------------------------------
// const Segmentation::InfoTagList InformationProxy::availableInformation() const
// {
//   if (m_elements.isEmpty())
//     return QStringList();
// 
//   ModelItemPtr item = indexPtr(m_elements.first());
//   return segmentationPtr(item)->availableInformations();
// }
// 


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
    if (!m_tags.isEmpty())
      beginInsertRows(QModelIndex(), startRow, endRow);

    for(auto acceptedItem : acceptedItems)
    {
      m_elements << acceptedItem;
    }

    if (!m_tags.isEmpty())
      endInsertRows();
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
      if (!m_tags.isEmpty())
        beginRemoveRows(QModelIndex(), removedIndexes.first().row(), removedIndexes.last().row());
      for(auto index : removedIndexes)
      {
        // We use start instead of row to avoid access to removed indices
        auto removedItem = itemAdapter(index);
        m_elements.removeOne(removedItem);
      }
      if (!m_tags.isEmpty())
        endRemoveRows();
    }
  }
}

//------------------------------------------------------------------------
void InformationProxy::sourceDataChanged(const QModelIndex& sourceTopLeft, const QModelIndex& sourceBottomRight)
{
  Q_ASSERT(sourceTopLeft == sourceBottomRight);
  if (sourceTopLeft.parent() == m_model->segmentationRoot())
  {
    auto item = itemAdapter(sourceTopLeft);
    auto segmentation = segmentationPtr(item);

    if (m_elements.contains(item) && !acceptSegmentation(segmentation))
    {
      int row = m_elements.indexOf(item);

      beginRemoveRows(QModelIndex(), row, row);
      m_elements.removeAt(row);
      endRemoveRows();

    } else if (!m_elements.contains(item) && acceptSegmentation(segmentation))
    {
      int row = m_elements.size();

      beginInsertRows(QModelIndex(), row, row);
      m_elements << item;
      endInsertRows();

    } else
    {
      emit dataChanged(mapFromSource(sourceTopLeft), mapFromSource(sourceBottomRight));
    }
  }
}


//------------------------------------------------------------------------
void InformationProxy::onProgessReported(int progress)
{
  auto task = dynamic_cast<InformationFetcher *>(sender());
  Q_ASSERT(task);

  auto firstColumn = index(task->Segmentation);
  auto lastColumn  = index(task->Segmentation, rowCount() - 1);

  emit dataChanged(firstColumn, lastColumn);
}

//------------------------------------------------------------------------
void InformationProxy::onTaskFininished()
{
  auto task = dynamic_cast<InformationFetcher *>(sender());
  Q_ASSERT(task);

  auto firstColumn = index(task->Segmentation);
  auto lastColumn  = index(task->Segmentation, rowCount() - 1);

  firstColumn = index(0, 0);
  lastColumn  = index(rowCount(), 0);

  emit dataChanged(firstColumn, lastColumn);
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
