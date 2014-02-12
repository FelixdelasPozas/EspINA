/*
 *    <one line to give the program's name and a brief idea of what it does.>
 *    Copyright (C) 2011  Jorge Peña <jorge.pena.pastor@gmail.com>
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

//----------------------------------------------------------------------------
// File:    ChannelProxy.h
// Purpose: Rearrange model items to group Channels by Samples
//----------------------------------------------------------------------------
#ifndef ESPINA_CHANNEL_PROXY_H
#define ESPINA_CHANNEL_PROXY_H


#include <QAbstractProxyModel>

#include <GUI/Model/ModelAdapter.h>

namespace EspINA
{
  // Forward declaration
  class ViewManager;

  /// Group Channels by Sample
  class EspinaGUI_EXPORT ChannelProxy
  : public QAbstractProxyModel
  {
    Q_OBJECT
  public:
    explicit ChannelProxy(ModelAdapterSPtr sourceModel, QObject* parent = 0);
    virtual ~ChannelProxy();

    virtual void setSourceModel(ModelAdapterSPtr sourceModel);

    virtual QVariant data(const QModelIndex& proxyIndex, int role = Qt::DisplayRole) const;

    virtual bool hasChildren(const QModelIndex& parent = QModelIndex()) const;
    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex& parent = QModelIndex()) const {return 1;}
    virtual QModelIndex parent(const QModelIndex& child) const;
    virtual QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const;

    virtual QModelIndex mapFromSource(const QModelIndex& sourceIndex) const;
    virtual QModelIndex mapToSource(const QModelIndex& proxyIndex) const;

    /// Drag & Drop support
    virtual Qt::ItemFlags flags(const QModelIndex& index) const;
    virtual Qt::DropActions supportedDropActions() const {return Qt::MoveAction;}
    virtual bool dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent);

    int numChannels(QModelIndex sampleIndex, bool recursive = false) const;
    int numSubSamples(QModelIndex sampleIndex) const;
    QModelIndexList channels(QModelIndex sampleIndex, bool recursive=false) const;

  signals:
    void channelsDragged(ChannelAdapterList sources, SampleAdapterPtr destination);

  protected slots:
    void sourceRowsInserted(const QModelIndex & sourceParent, int start, int end);
    void sourceRowsAboutToBeRemoved(const QModelIndex & sourceParent, int start, int end);
    void sourceRowsRemoved(const QModelIndex & sourceParent, int start, int end);
    void sourceDataChanged(const QModelIndex& sourceTopLeft, const QModelIndex& sourceBottomRight);
    void sourceModelReset();

  protected:
    bool indices(const QModelIndex &topLeft, const QModelIndex &bottomRight, QModelIndexList &result);
    //   QModelIndexList indices(const QModelIndex& parent, int start, int end);
    QModelIndexList proxyIndices(const QModelIndex& parent, int start, int end) const;
    int numChannels(SampleAdapterPtr sample) const;
    int numSubSamples(SampleAdapterPtr sample) const;

  private:
    ModelAdapterSPtr m_model;

    SampleAdapterList m_samples;
    mutable QMap<SampleAdapterPtr, ItemAdapterList> m_channels;
  };

} // namespace EspINA

#endif // ESPINA_CHANNEL_PROXY_H
