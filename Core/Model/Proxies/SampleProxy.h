/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2011  Jorge Peña <jorge.pena.pastor@gmail.com>

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

//----------------------------------------------------------------------------
// File:    SampleProxy.h
// Purpose: Rearrange model items to group Segmentations by Samples
//----------------------------------------------------------------------------
#ifndef SAMPLEPROXY_H
#define SAMPLEPROXY_H

#include <QAbstractProxyModel>

#include <Core/EspinaTypes.h>

namespace EspINA
{
  /// Group Segmentations by Sample
  class SampleProxy
  : public QAbstractProxyModel
  {
    Q_OBJECT
  public:
    SampleProxy(QObject* parent = 0);
    virtual ~SampleProxy();

    virtual void setSourceModel(EspinaModelPtr sourceModel);

    virtual QVariant data(const QModelIndex& proxyIndex, int role = Qt::DisplayRole) const;

    virtual bool hasChildren(const QModelIndex& parent = QModelIndex()) const;
    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex& parent = QModelIndex()) const {return 1;}
    virtual QModelIndex parent(const QModelIndex& child) const;
    virtual QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const;

    virtual QModelIndex mapFromSource(const QModelIndex& sourceIndex) const;
    virtual QModelIndex mapToSource(const QModelIndex& proxyIndex) const;

    int numSegmentations(QModelIndex sampleIndex, bool recursive = false) const;
    int numSubSamples(QModelIndex sampleIndex) const;
    QModelIndexList segmentations(QModelIndex sampleIndex, bool recursive=false) const;

  protected slots:
    void sourceRowsInserted(const QModelIndex & sourceParent, int start, int end);
    void sourceRowsAboutToBeRemoved(const QModelIndex & sourceParent, int start, int end);
    void sourceRowsRemoved(const QModelIndex & sourceParent, int start, int end);
    void sourceDataChanged(const QModelIndex& sourceTopLeft, const QModelIndex& sourceBottomRight);

  protected:
    bool indices(const QModelIndex &topLeft, const QModelIndex &bottomRight, QModelIndexList &result);
    //   QModelIndexList indices(const QModelIndex& parent, int start, int end);
    QModelIndexList proxyIndices(const QModelIndex& parent, int start, int end) const;
    void updateSegmentations() const;
    int numSegmentations(SamplePtr sample) const;
    int numSubSamples(SamplePtr sample) const;

  private:
    EspinaModelPtr m_model;
    SampleList m_samples;
    mutable QMap<SamplePtr, ModelItemList> m_segmentations;
  };

} // namespace EspINA

#endif // SAMPLEPROXY_H
