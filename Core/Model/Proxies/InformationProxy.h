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


#ifndef INFORMATIONPROXY_H
#define INFORMATIONPROXY_H

#include <QAbstractProxyModel>

#include <Core/EspinaTypes.h>
#include <Core/Model/EspinaModel.h>
#include <Core/Model/Segmentation.h>

#include <QStringList>

namespace EspINA
{

  class InformationProxy
  : public QAbstractProxyModel
  {
    Q_OBJECT
  public:
    explicit InformationProxy();
    virtual ~InformationProxy();

    virtual void setSourceModel(EspinaModel *sourceModel);

    virtual QModelIndex mapFromSource(const QModelIndex& sourceIndex) const;
    virtual QModelIndex mapToSource(const QModelIndex& proxyIndex) const;

    virtual int columnCount(const QModelIndex& parent = QModelIndex()) const;
    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
    virtual QModelIndex parent(const QModelIndex& child) const;
    virtual QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const;

    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    virtual QVariant data(const QModelIndex& proxyIndex, int role = Qt::DisplayRole) const;

    void setTaxonomy(const QString &qualifiedName);
    QString taxonomy() const { return m_taxonomy; }

    void setFilter(const SegmentationList *filter);

    void setInformationTags(const Segmentation::InfoTagList tags);
    const QStringList informationTags() const {return m_tags;}
    //const Segmentation::InfoTagList availableInformation() const;

  protected slots:
    void sourceRowsInserted(const QModelIndex & sourceParent, int start, int end);
    void sourceRowsAboutToBeRemoved(const QModelIndex & sourceParent, int start, int end);
    void sourceDataChanged(const QModelIndex& sourceTopLeft, const QModelIndex& sourceBottomRight);
    void sourceModelReset();

  private:
    bool acceptSegmentation(const SegmentationPtr segmentation) const;

  private:
    EspinaModel *m_model;

    QString                   m_taxonomy;
    const SegmentationList   *m_filter;
    Segmentation::InfoTagList m_tags;

    QList<ModelItemPtr> m_elements;
  };

} // namespace EspINA

#endif // INFORMATIONPROXY_H
