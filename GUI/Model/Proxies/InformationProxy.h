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


#ifndef ESPINA_INFORMATION_PROXY_H
#define ESPINA_INFORMATION_PROXY_H

#include "EspinaCore_Export.h"

#include <QAbstractProxyModel>

#include <GUI/Model/ModelAdapter.h>

#include <QStringList>

namespace EspINA
{

  class EspinaCore_EXPORT InformationProxy
  : public QAbstractProxyModel
  {
    Q_OBJECT

    class InformationFetcher;
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

    void setInformationTags(const SegmentationExtension::InfoTagList tags);

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

  private:
    ModelAdapterSPtr m_model;
    QString          m_category;

    const SegmentationAdapterList     *m_filter;
    SegmentationExtension::InfoTagList m_tags;

    ItemAdapterList m_elements;

    SchedulerSPtr m_scheduler;
    mutable QMap<SegmentationAdapterPtr, InformationFetcherSPtr> m_pendingInformation;
  };
} // namespace EspINA

#endif // INFORMATIONPROXY_H
