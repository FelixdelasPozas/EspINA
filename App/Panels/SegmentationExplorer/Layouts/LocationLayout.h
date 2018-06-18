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

#ifndef APP_PANELS_SEGMENTATIONEXPLORER_LAYOUTS_LOCATIONLAYOUT_H_
#define APP_PANELS_SEGMENTATIONEXPLORER_LAYOUTS_LOCATIONLAYOUT_H_

// ESPINA
#include <App/Panels/SegmentationExplorer/SegmentationExplorerLayout.h>
#include <GUI/Model/Proxies/LocationProxy.h>

class QItemDelegate;
class QMenu;

namespace ESPINA
{
  /** \class ClassificationLayout
   * \brief Implements the layout in the segmentation explorer based on the classification order.
   *
   */
  class LocationLayout
  : public SegmentationExplorer::Layout
  {
      Q_OBJECT

      /** \class SortFilter
       * \brief Implements the sorting algorithm for segmentations in the layout.
       *
       */
      class LocationSortFilter
      : public SegmentationFilterProxyModel
      {
        protected:
          virtual bool lessThan(const QModelIndex& left, const QModelIndex& right) const override;
      };

    public:
      /** \brief LocationLayout class constructor.
       * \param[in] view QTreeView raw pointer.
       * \param[in] context ESPINA context
       *
       */
      explicit LocationLayout(CheckableTreeView *view,
                              Support::Context  &context);

      /** \brief LocationLayout class virtual destructor.
       *
       */
      virtual ~LocationLayout();

      virtual void createSpecificControls(QHBoxLayout *specificControlLayout) override;

      virtual QAbstractItemModel* model() override
      { return m_sort.get(); }

      virtual ItemAdapterPtr item(const QModelIndex& index) const override;

      virtual QModelIndex index(ItemAdapterPtr item) const override;

      virtual void setFilterRegExp(const QString &regExp) override
      { m_sort->setFilterRegExp(regExp); }

      virtual void contextMenu(const QPoint &pos) override;

      virtual void deleteSelectedItems() override;

      virtual void showSelectedItemsInformation() override;

      virtual bool hasInformationToShow() override;

      virtual QItemDelegate *itemDelegate() const;

    public slots:
      virtual void updateSelection();

    private slots:
      /** \brief Changes the given list of segmentations to have the given stack as the input.
       * \param[in] segmentations SegmentationAdapter pointer list.
       * \param[in] stack Channel adapter pointer.
       *
       */
      void segmentationsDropped(SegmentationAdapterList segmentations,ChannelAdapterPtr stack);

      /** \brief Selects all the segmentations from a stack.
       *
       */
      void selectAllFromStack();

      /** \brief Moves selected segmentations to the selected stack.
       *
       */
      void moveToStack();

      /** \brief Moves all segmentations of the model to the selected stack.
       *
       */
      void moveAllToStack();

      /** \brief Select all orphaned segmentations.
       *
       */
      void selectAllOrphans();

    private:
      /** \brief Updates the class internal lists of selected segmentations and stacks.
       *
       */
      void updateSelectedItems();

      /** \brief Helper method to create an additional entry in the default context menu.
       * \param[in] menu Context menu to create the entry at the beginning.
       *
       */
      void createChangeStackEntry(QMenu *menu);

      using LayoutProxy = ESPINA::GUI::Model::Proxy::LocationProxy;

      std::shared_ptr<LocationSortFilter> m_sort;           /** model filter and sorter.                                 */
      std::shared_ptr<LayoutProxy>        m_proxy;          /** proxy model.                                             */
      SegmentationAdapterList             m_selectedSegs;   /** list of currently selected segmentations.                */
      ChannelAdapterList                  m_selectedStacks; /** list of currently selected stacks.                       */
      bool                                m_orphanSelected; /** true if the orphan 'stack' is selected, false otherwise. */
      QItemDelegate                      *m_delegate;       /** item delegate.                                           */
  };

} // namespace ESPINA

#endif // APP_PANELS_SEGMENTATIONEXPLORER_LAYOUTS_LOCATIONLAYOUT_H_
