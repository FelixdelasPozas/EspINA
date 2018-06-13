/*

 Copyright (C) 2018 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

 This file is part of ESPINA.

 ESPINA is free software: you can redistribute it and/or modify
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

#ifndef GUI_MODEL_PROXIES_LOCATIONPROXY_H_
#define GUI_MODEL_PROXIES_LOCATIONPROXY_H_

// ESPINA
#include <GUI/Model/ModelAdapter.h>

// Qt
#include <QAbstractProxyModel>

namespace ESPINA
{
  namespace GUI
  {
    namespace Model
    {
      namespace Proxy
      {
        /** \class LocationProxy
         * \brief Groups segmentations by the stack of origin.
         *
         */
        class LocationProxy
        : public QAbstractProxyModel
        {
            Q_OBJECT
          public:
            /** \brief LocationProxy class constructor.
             * \param[in] sourceModel model adapter smart pointer.
             * \param[in] viewState Application state of the views object reference.
             * \param[in] parent raw pointer of the parent of this object.
             *
             */
            explicit LocationProxy(ModelAdapterSPtr sourceModel, GUI::View::ViewState &viewState, QObject *parent = nullptr);

            /** \brief LocationProxy class virtual destructor.
             *
             */
            virtual ~LocationProxy();

            virtual void setSourceModel(ModelAdapterSPtr sourceModel);

            virtual QVariant data(const QModelIndex& proxyIndex, int role = Qt::DisplayRole) const override;

            virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

            virtual bool hasChildren(const QModelIndex& parent = QModelIndex()) const override;

            virtual int rowCount(const QModelIndex& parent = QModelIndex()) const override;

            virtual int columnCount(const QModelIndex& parent = QModelIndex()) const override {return 1;}

            virtual QModelIndex parent(const QModelIndex& child) const override;

            virtual QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;

            virtual QModelIndex mapFromSource(const QModelIndex& sourceIndex) const override;

            virtual QModelIndex mapToSource(const QModelIndex& proxyIndex) const override;

            virtual Qt::ItemFlags flags(const QModelIndex& index) const override;

            virtual Qt::DropActions supportedDropActions() const override {return Qt::MoveAction;}

            virtual bool dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) override;

            /** \brief Helper method that returns all the segmentations identified to be in the given stack.
             * \param[in] stack Stack adapter object.
             *
             */
            const SegmentationAdapterList segmentationsOf(const ChannelAdapterPtr stack) const;

            /** \brief Helper method that returns the stack of a given segmentation.
             * \param[in] segmentation Segmentation adapter object.
             *
             */
            const ChannelAdapterPtr stackOf(const SegmentationAdapterPtr segmentation) const;

            void printContents();

          signals:
            void segmentationsDropped(SegmentationAdapterList sources, ChannelAdapterPtr stack);

          private slots:
            /** \brief Inserts rows in the model given the parent in the source model and the interval.
             * \param[in] sourceParent model index of the parent item in the source model.
             * \param[in] start start value.
             * \param[in] end end value.
             *
             */
            void sourceRowsInserted(const QModelIndex & sourceParent, int start, int end);

            /** \brief Removes rows in the model given the parent in the source model and the interval, before the rows being removed in the source model.
             * \param[in] sourceParent model index of the parent item in the source model.
             * \param[in] start start value.
             * \param[in] end end value.
             *
             */
            void sourceRowsAboutToBeRemoved(const QModelIndex & sourceParent, int start, int end);

            /** \brief Removes rows in the model given the parent in the source model and the interval.
             * \param[in] sourceParent model index of the parent item in the source model.
             * \param[in] start start value.
             * \param[in] end end value.
             *
             */
            void sourceRowsRemoved(const QModelIndex & sourceParent, int start, int end);

            /** \brief Moves rows in the model given the parent in the source model and the interval, before the rows being moved in the source model.
             * \param[in] sourceParent model index of the parent item in the source model.
             * \param[in] sourceStart start value.
             * \param[in] sourceEnd end value.
             * \param[in] destinationParent model index of the destination in the source model.
             * \param[in] destinationRow destination row.
             *
             */
            void sourceRowsAboutToBeMoved(const QModelIndex &sourceParent, int sourceStart, int sourceEnd, const QModelIndex & destinationParent, int destinationRow);

            /** \brief Moves rows in the model given the parent in the source model and the interval.
             * \param[in] sourceParent model index of the parent item in the source model.
             * \param[in] sourceStart start value.
             * \param[in] sourceEnd end value.
             * \param[in] destinationParent model index of the destination in the source model.
             * \param[in] destinationRow destination row.
             *
             */
            void sourceRowsMoved(const QModelIndex &sourceParent, int sourceStart, int sourceEnd, const QModelIndex & destinationParent, int destinationRow);

            /** \brief Updates the data in the model when it changes in the source model.
             * \param[in] sourceTopLeft source model index top left.
             * \param[in] sourceBottomRight source model index bottom right.
             */
            void sourceDataChanged(const QModelIndex &sourceTopLeft, const QModelIndex &sourceBottomRight);

            /** \brief Resets the model of the proxy.
             *
             */
            void sourceModelReset();

          private:
            /** \brief Changes the checkstate value of the given index and associated item (and possibly children) to the given value.
             * \param[in] index QModelIndex object.
             * \param[in] value true to make it visible and false otherwise.
             *
             */
            void changeIndexVisibility(const QModelIndex &index, bool value);

            /** \brief Changes the checkstate value of the given index to the given value.
             * \param[in] index QModelIndex object.
             * \param[in] value true to make it visible and false otherwise.
             *
             */
            void changeParentCheckStateRole(const QModelIndex &index, bool value);

            /** \brief Invalidates the representations of the given index and its children.
             * \param[in] index QModelIndex object.
             *
             */
            void notifyModifiedRepresentations(const QModelIndex &index);

            /** \brief Group all segmentations of the base model between start and end by the stacks they belong to.
             *
             */
            const QMap<ChannelAdapterSPtr, ItemAdapterList> groupSegmentationsByStack(int start, int end);

            /** \brief Returns the QModelIndex in the model of the given stack.
             * \param[in] stack Stack adapter smart pointer.
             */
            const QModelIndex channelIndex(const ChannelAdapterPtr stack);

            /** \brief Retuns the list of QModelIndex the the selected interval in the parameter.
             * \param[in] topLeft top left index of the selection.
             * \param[in] bottomRight bottom right index of the selection.
             * \param[out] result list of QModelIndex of the selection.
             *
             */
            bool indices(const QModelIndex& topLeft, const QModelIndex& bottomRight, QModelIndexList& result);

          private:
            ModelAdapterSPtr                                 m_model;         /** Model adapter.                                       */
            GUI::View::ViewState                            &m_viewState;     /** Applications state of the views reference.           */
            QMap<ChannelAdapterPtr, Qt::CheckState>          m_visible;       /** Maps the stack with the visibility state.            */
            QMap<ChannelAdapterPtr, SegmentationAdapterList> m_segmentations; /** Maps the stack with the segmentations located in it. */
        };
      
      } // namespace Proxy
    } // namespace Model
  } // namespace GUI
} // namespace ESPINA

#endif // GUI_MODEL_PROXIES_LOCATIONPROXY_H_
