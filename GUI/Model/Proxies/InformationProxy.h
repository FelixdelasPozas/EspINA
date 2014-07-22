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


#ifndef ESPINA_INFORMATION_PROXY_H
#define ESPINA_INFORMATION_PROXY_H

// ESPINA
#include <Core/MultiTasking/Task.h>
#include <GUI/Model/ModelAdapter.h>

// Qt
#include <QAbstractProxyModel>
#include <QStringList>

const ESPINA::SegmentationExtension::InfoTag NAME_TAG     = QObject::tr("Name");
const ESPINA::SegmentationExtension::InfoTag CATEGORY_TAG = QObject::tr("Category");

namespace ESPINA
{

  class EspinaCore_EXPORT InformationProxy
  : public QAbstractProxyModel
  {
    Q_OBJECT

  protected:
    class InformationFetcher
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

        m_tags.removeOne(NAME_TAG);
        m_tags.removeOne(CATEGORY_TAG);

        bool ready = true;
        for (auto tag : m_tags)
        {
          ready &= Segmentation->isInformationReady(tag);

          if (!ready) break;
        }

        setFinished(ready);
      }

    public:
      int currentProgress() const
      { return m_progress; }

    protected:
      virtual void run()
      {
        for (int i = 0; i < m_tags.size(); ++i)
        {
          if (!canExecute()) break;

          auto tag = m_tags[i];
          if (tag != NAME_TAG && tag != CATEGORY_TAG)
          {
            if (!Segmentation->isInformationReady(tag))
            {
              setWaiting(true);
              Segmentation->information(tag);
              setWaiting(false);
              if (!canExecute()) break;
            }
          }

          m_progress = (100.0*i)/m_tags.size();
          emit progress(m_progress);
        }
      }

    protected:
      SegmentationAdapterPtr Segmentation;
      const SegmentationExtension::InfoTagList m_tags;
      int   m_progress;
    };

    using InformationFetcherSPtr = std::shared_ptr<InformationFetcher>;

  public:
    explicit InformationProxy(SchedulerSPtr scheduler);
    virtual ~InformationProxy();

    virtual void setSourceModel(ModelAdapterSPtr sourceModel);

    virtual QModelIndex mapFromSource(const QModelIndex& sourceIndex) const;

    virtual QModelIndex mapToSource(const QModelIndex& proxyIndex) const;

    virtual int columnCount(const QModelIndex& parent = QModelIndex()) const;

    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;

    virtual QModelIndex parent(const QModelIndex& child) const;

    virtual QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const;

    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;

    virtual QVariant data(const QModelIndex& proxyIndex, int role = Qt::DisplayRole) const;

    void setCategory(const QString &classificationName);

    QString category() const
    { return m_category; }

    void setFilter(const SegmentationAdapterList *filter);

    virtual void setInformationTags(const SegmentationExtension::InfoTagList tags);

    const QStringList informationTags() const
    { return m_tags; }
    //const Segmentation::InfoTagList availableInformation() const;

    ItemAdapterList displayedItems() const
    { return m_elements; }

    int progress() const;

  signals:
    void informationProgress();

  protected slots:
    void sourceRowsInserted(const QModelIndex & sourceParent, int start, int end);

    void sourceRowsAboutToBeRemoved(const QModelIndex & sourceParent, int start, int end);

    void sourceDataChanged(const QModelIndex& sourceTopLeft, const QModelIndex& sourceBottomRight);

    void sourceModelReset();

    void onProgessReported(int progress);

    void onTaskFininished();

  private:
    bool acceptSegmentation(const SegmentationAdapterPtr segmentation) const;

    QModelIndex index(const ItemAdapterPtr segmentation, int col = 0);

  protected:
    SegmentationExtension::InfoTagList m_tags;
    mutable QMap<SegmentationAdapterPtr, InformationFetcherSPtr> m_pendingInformation;
    SchedulerSPtr m_scheduler;

  private:
    ModelAdapterSPtr m_model;
    QString          m_category;

    const SegmentationAdapterList     *m_filter;

    ItemAdapterList m_elements;
  };
} // namespace ESPINA

#endif // INFORMATIONPROXY_H
