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

#ifndef ESPINA_SEGMENTATION_EXPLORER_LAYOUT_H
#define ESPINA_SEGMENTATION_EXPLORER_LAYOUT_H

// ESPINA
#include "SegmentationExplorer.h"
#include <GUI/Widgets/CheckableTreeView.h>
#include <Support/Factory/FilterRefinerFactory.h>
#include <Support/Representations/RepresentationFactory.h>

// Qt
#include <QItemDelegate>
#include <QSortFilterProxyModel>

namespace ESPINA
{
  class SegmentationInspector;

  /** \class SegmentationFilterProxyModel
   * \brief Model filter class to filter objects to show, in this case using a regular expression in name and tags.
   *
   */
  class SegmentationFilterProxyModel
  : public QSortFilterProxyModel
  {
    public:
      /** \brief SegmentationFilterProxyModel class constructor.
       * \param[in] parent parent qobject raw pointer.
       *
       */
      explicit SegmentationFilterProxyModel(QObject *parent = nullptr);

    protected:
      virtual bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;
  };

  /** \class Layout
   * \brief Segmentation explorer layout that classifies segmentations based on their category.
   *
   */
  class SegmentationExplorer::Layout
  : public QObject
  , protected Support::WithContext
  {
      Q_OBJECT
    protected:
      static const QString SEGMENTATION_MESSAGE;
      static const QString RECURSIVE_MESSAGE;
      static const QString MIXED_MESSAGE;

    public:
      /** \brief Layout class constructor.
       * \param[in] view QTreeView raw pointer.
       * \param[in] context ESPINA context
       *
       */
      explicit Layout(CheckableTreeView              *view,
                      Support::Context               &context);

      /** \brief Layout class virtual destructor.
       *
       */
      virtual ~Layout()
      {};

      /** \brief Creates specific GUI controls for the layout.
       *
       */
      virtual void createSpecificControls(QHBoxLayout *specificControlLayout) = 0;

      /** \brief Returns the QAbstractItemModel raw pointer.
       *
       */
      virtual QAbstractItemModel *model()
      {return getModel().get(); }

      /** \brief Returns the ItemAdapter raw pointer of the QModelIndex passed as paramenter in this layout.
       * \param[in] index, const QModelIndex reference of the item.
       *
       */
      virtual ItemAdapterPtr item(const QModelIndex &index) const;

      /** \brief Returns the QModelIndex associated to the ItemAdapter raw pointer passed as parameter in this layout.
       * \param[in] item, ItemAdapter raw pointer of the item.
       */
      virtual QModelIndex index(ItemAdapterPtr item) const
      { return getModel()->index(item); }

      /** \brief Sets the regular expresion for the filter.
       * \param[in] regExpr, regular expresion to use as filter expression.
       *
       */
      virtual void setFilterRegExp(const QString &regExp) = 0;

      /** \brief Shows the context menu in the point passed as parameter.
       * \param[in] pos, place to show the layout context menu.
       */
      virtual void contextMenu(const QPoint &pos) = 0;

      /** \brief Deletes selected items in the layout.
       *
       */
      virtual void deleteSelectedItems() = 0;

      /** \brief Show information for the selected items in the layout.
       *
       */
      virtual void showSelectedItemsInformation() = 0;

      /** \brief Returns true if the layout has information to show about the items.
       *
       */
      virtual bool hasInformationToShow() = 0;

      /** \brief Returns the QItemDelegate pointer of the layout.
       *
       */
      virtual QItemDelegate *itemDelegate() const = 0;

      using SegmentationInspectorKey = QString;

      /** \brief Converts a list of segmentation to a unique string key.
       * \param[in] segmentations, list of segmentation adapter raw pointers.
       *
       */
      static SegmentationInspectorKey toKey(const SegmentationAdapterList segmentations);

      /** \brief Resets the layout.
       *
       */
      virtual void reset();

      /** \brief Sets the layout as active.
       * \param[in] value True to set the layout as the active one and false otherwise.
       *
       */
      virtual void setActive(const bool value)
      { m_active = value; }

      /** \brief Returns true if the layout is active and false otherwise.
       *
       */
      inline const bool isActive() const
      { return m_active; }

    public slots:
      /** \brief Updates the model and the layout specific controls.
       *
       */
      virtual void updateSelection() = 0;

    protected:
      /** \brief Deletes the segmentations from the model.
       * \param[in] segmentations, list of segmentation adapter raw pointers of the items to delete.
       *
       */
      void deleteSegmentations(SegmentationAdapterList segmentations);

      /** \brief Opens a SegmentationInspector dialog to inspect the selected segmentations.
       * \param[in] segmentations, list of segmentation adapter raw pointers of the selected items.
       */
      void showSegmentationProperties(SegmentationAdapterList segmentations);

      /** \brief Returns the list of QModelIndex that are childs of the specified QModelIndex.
       * \param[in] index const QModelIndex reference of the parent.
       * \param[in] recursive true if the return value includes the childs of the childs.
       *
       */
      QModelIndexList indices(const QModelIndex &index, bool recursive=false);

    protected slots:
      /** \brief Deletes the pointer of the segmentation inspector from the pointers dialog QMap.
       *
       */
      void releaseInspectorResources(SegmentationInspector *inspector);

      /** \brief Updates the inspector key when it changes the segmentations being inspected.
       *
       */
      void onInspectorUpdated();

    protected:
      CheckableTreeView                                      *m_view;       /** view of the model.                                */
      QMap<SegmentationInspectorKey, SegmentationInspector *> m_inspectors; /** map of segmentation inspectors.                   */
      bool                                                    m_active;     /** true if is the active layout and false otherwise. */
  };

} // namespace ESPINA

#endif // SEGMENTATIONEXPLORERLAYOUT_H
