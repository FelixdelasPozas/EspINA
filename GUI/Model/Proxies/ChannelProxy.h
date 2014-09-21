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

#ifndef ESPINA_CHANNEL_PROXY_H
#define ESPINA_CHANNEL_PROXY_H

#include <QAbstractProxyModel>

#include <GUI/Model/ModelAdapter.h>

namespace ESPINA
{
  class ViewManager;

  /** \class ChannelProxy.
   * \brief Rearrange model items to group Channels by Samples.
   */
  class EspinaGUI_EXPORT ChannelProxy
  : public QAbstractProxyModel
  {
    Q_OBJECT
  public:
  	/** brief ChannelProxy class constructor.
  	 * \param[in] sourceModel, model adapter smart pointer.
  	 * \param[in] parent, raw pointer of the parent of this object.
  	 *
  	 */
    explicit ChannelProxy(ModelAdapterSPtr sourceModel, QObject* parent = 0);

  	/** brief ChannelProxy class destructor.
  	 *
  	 */
    virtual ~ChannelProxy();

  	/** brief Sets the source model for the proxy.
  	 *
  	 */
    virtual void setSourceModel(ModelAdapterSPtr sourceModel);

  	/** brief Overrides QAbstractProxyModel::data().
  	 *
  	 */
    virtual QVariant data(const QModelIndex& proxyIndex, int role = Qt::DisplayRole) const override;

  	/** brief Overrides QAbstractProxyModel::hasChildren().
  	 *
  	 */
    virtual bool hasChildren(const QModelIndex& parent = QModelIndex()) const override;

  	/** brief Implements QAbstractProxyModel::rowCount().
  	 *
  	 */
    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;

  	/** brief Implements QAbstractProxyModel::columnCount().
  	 *
  	 */
    virtual int columnCount(const QModelIndex& parent = QModelIndex()) const {return 1;}

    /** brief Implements QAbstractProxyModel::parent().
  	 *
  	 */
    virtual QModelIndex parent(const QModelIndex& child) const;

    /** brief Implements QAbstractProxyModel::index().
  	 *
  	 */
    virtual QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const;

    /** brief Implements QAbstractProxyModel::mapFromSource().
  	 *
  	 */
    virtual QModelIndex mapFromSource(const QModelIndex& sourceIndex) const;

    /** brief Implements QAbstractProxyModel::mapToSource().
  	 *
  	 */
    virtual QModelIndex mapToSource(const QModelIndex& proxyIndex) const;

  	/** brief Overrides QAbrstractProxyModel::flags().
  	 *
  	 * Drag & Drop support
  	 *
  	 */
    virtual Qt::ItemFlags flags(const QModelIndex& index) const override;

  	/** brief Overrides QAbrstractProxyModel::supportedDropActions().
  	 *
  	 * Drag & Drop support
  	 *
  	 */
    virtual Qt::DropActions supportedDropActions() const override
    {return Qt::MoveAction;}

  	/** brief Overrides QAbrstractProxyModel::dropMimeData().
  	 *
  	 * Drag & Drop support
  	 *
  	 */
    virtual bool dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) override;

  	/** brief Returns the number of channels of the given index in the proxy model.
  	 * \param[in] channelIndex, proxy model index.
  	 * \param[in] recursive, include the sub-channels of the given index.
  	 *
  	 */
    int numChannels(QModelIndex channelIndex, bool recursive = false) const;

  	/** brief Returns the number of samples of the given index in the proxy model.
  	 * \param[in] sampleIndex, proxy model index.
  	 * \param[in] recursive, include the sub-samples of the given index.
  	 *
  	 */
    int numSubSamples(QModelIndex sampleIndex) const;

  	/** brief Returns the list of proxy indexes for the channels of a given proxy model index.
  	 * \param[in] sampleIndex, proxy model index.
  	 * \param[in] recursive, include the sub-channels of the given index.
  	 *
  	 */
    QModelIndexList channels(QModelIndex sampleIndex, bool recursive=false) const;

  signals:
    void channelsDragged(ChannelAdapterList sources, SampleAdapterPtr destination);

  protected slots:
		/** brief Insert rows into the proxy model after rows have been inserted in the source model.
		 * \param[in] sourceParent, source model index.
		 * \param[in] start, interval start.
		 * \param[in] end, interval end.
		 *
		 */
  	void sourceRowsInserted(const QModelIndex & sourceParent, int start, int end);

		/** brief Removes rows into the proxy model before rows are going to be removed in the source model.
		 * \param[in] sourceParent, source model index.
		 * \param[in] start, interval start.
		 * \param[in] end, interval end.
		 *
		 */
  	void sourceRowsAboutToBeRemoved(const QModelIndex & sourceParent, int start, int end);

		/** brief Removes rows into the proxy model after rows have been removed from the source model.
		 * \param[in] sourceParent, source model index.
		 * \param[in] start, interval start.
		 * \param[in] end, interval end.
		 *
		 */
  	void sourceRowsRemoved(const QModelIndex & sourceParent, int start, int end);

  	/** brief Moves rows of the proxy model before rows will be moved in the source model.
		 * \param[in] sourceParent, source model index.
		 * \param[in] sourceStart, interval start.
		 * \param[in] sourceEnd, interval end.
		 * \param[in] destinationParent, source model index destination of the move.
		 * \param[in] destinationRow, destination row.
  	 *
  	 */
  	void sourceRowsAboutToBeMoved(const QModelIndex &sourceParent, int sourceStart, int sourceEnd, const QModelIndex & destinationParent, int destinationRow);

  	/** brief Moves rows of the proxy model after rows have been moved in the source model.
		 * \param[in] sourceParent, source model index.
		 * \param[in] sourceStart, interval start.
		 * \param[in] sourceEnd, interval end.
		 * \param[in] destinationParent, source model index destination of the move.
		 * \param[in] destinationRow, destination row.
  	 *
  	 */
  	void sourceRowsMoved(const QModelIndex &sourceParent, int sourceStart, int sourceEnd, const QModelIndex & destinationParent, int destinationRow);

  	/** brief Updates the proxy model data when the source model data changes.
  	 * \param[in] sourceTopLeft, source model index top left selection.
  	 * \param[in] sourceBottomRight, source model index bottom right selection.
  	 *
  	 */
  	void sourceDataChanged(const QModelIndex& sourceTopLeft, const QModelIndex& sourceBottomRight);

  	/** brief Resets the proxy model.
  	 *
  	 */
  	void sourceModelReset();

  protected:
  	/** brief Returns the list of indexes in the proxy model of the selection.
  	 * \param[in] topLeft, top left model index.
  	 * \param[in] bottomRight, bottom right model index.
  	 * \param[out] result, list of proxy model indexes.
  	 *
  	 */
  	bool indices(const QModelIndex &topLeft, const QModelIndex &bottomRight, QModelIndexList &result);

  	/** brief Returns the list of proxy indexes of the selection.
  	 * \param[in] parent, proxy model index parent of the selection.
  	 * \param[in] start, interval start.
  	 * \param[in] end, interval end.
  	 *
  	 */
    QModelIndexList proxyIndices(const QModelIndex& parent, int start, int end) const;

  	/** brief Returns the number of channels of the given sample.
  	 * \param[in] sample, sample adapter raw pointer.
  	 *
  	 */
    int numChannels(SampleAdapterPtr sample) const;

  	/** brief Returns the number of sub-samples of the given sample.
  	 * \param[in] sample, sample adapter raw pointer.
  	 *
  	 */
    int numSubSamples(SampleAdapterPtr sample) const;

  	/** brief Returns the sample related to the given channel.
  	 * \param[in] channel, channel adapter raw pointer.
  	 *
  	 */
    SampleAdapterPtr sample(ChannelAdapterPtr channel) const;

  private:
    ModelAdapterSPtr m_model;

    SampleAdapterList m_samples;
    mutable QMap<SampleAdapterPtr, ItemAdapterList> m_channels;
  };

} // namespace ESPINA

#endif // ESPINA_CHANNEL_PROXY_H
