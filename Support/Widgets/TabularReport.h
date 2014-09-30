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

#ifndef ESPINA_TABULAR_REPORT_H
#define ESPINA_TABULAR_REPORT_H

#include "Support/EspinaSupport_Export.h"

// ESPINA
#include <Support/ViewManager.h>
#include <GUI/Model/ModelAdapter.h>

// Qt
#include <QWidget>
#include <QSortFilterProxyModel>
#include <QAbstractItemView>
#include <QVBoxLayout>

class QPushButton;
class QTableView;

namespace ESPINA
{
  class EspinaSupport_EXPORT TabularReport
  : public QAbstractItemView
  {
    Q_OBJECT

  protected:
    class Entry;

  public:
    /** \brief TabularReport class cosntructor.
     * \param[in] factory, model factory smart pointer.
     * \param[in] viewManager, view manager smart pointer.
     * \param[in] parent, raw pointer of the QWidget parent of this one.
     * \param[in] flags, window flags.
     *
     */
    explicit TabularReport(ModelFactorySPtr factory,
                           ViewManagerSPtr  viewManager,
                           QWidget         *parent = nullptr,
                           Qt::WindowFlags  flags = Qt::WindowFlags{Qt::WindowNoState});

    /** \brief TabularReport class virtual destructor.
     *
     */
    virtual ~TabularReport();

    /** \brief Implements QAbstractViewItem::horizontalOffset().
     *
     */
    virtual int horizontalOffset() const
    { return 0;}

    /** \brief Implements QAbstractViewItem::indexAt().
     *
     */
    virtual QModelIndex indexAt(const QPoint &point) const
    { return QModelIndex(); }

    /** \brief Implements QAbstractViewItem::isIndexHidden().
     *
     */
    virtual bool isIndexHidden(const QModelIndex &index) const
    { return false; }

    /** \brief Implements QAbstractViewItem::moveCursor().
     *
     */
    virtual QModelIndex moveCursor(CursorAction cursorAction, Qt::KeyboardModifiers modifiers)
    { return QModelIndex(); }

    /** \brief Implements QAbstractViewItem::scrollTo().
     *
     */
    virtual void scrollTo(const QModelIndex &index, ScrollHint hint = EnsureVisible)
    {}

    /** \brief Implements QAbstractViewItem::setSelection().
     *
     */
    virtual void setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags command)
    {}

    /** \brief Implements QAbstractViewItem::verticalOffset().
     *
     */
    virtual int verticalOffset() const
    { return 0; }

    /** \brief Implements QAbstractViewItem::visualRect().
     *
     */
    virtual QRect visualRect(const QModelIndex &index) const
    { return QRect(); }

    /** \brief Implements QAbstractViewItem::visualRegionForSelection().
     *
     */
    virtual QRegion visualRegionForSelection(const QItemSelection &selection) const
    {return QRegion();}

    /** \brief Sets the model to be used.
     * \param[in] model, model adapter smart pointer.
     */
    virtual void setModel(ModelAdapterSPtr model);

    /** \brief Sets the filter of the model.
     * \param[in] segmentations, list of segmentation adapters to be show.
     *
     * Only display segmentations' information. If segmentations is empty, then
     * all segmentations' information is displayed.
     *
     */
    virtual void setFilter(SegmentationAdapterList segmentations);

  protected:
    /** \brief Overrides QAbstractItemView::dataChanged().
     *
     */
    virtual void dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight) override;

    /** \brief Overrides QAbstractItemView::rowsInserted().
     *
     */
    virtual void rowsInserted(const QModelIndex &parent, int start, int end) override;

    /** \brief Overrides QAbstractItemView::event().
     *
     */
    virtual bool event(QEvent *event) override;

    /** \brief Overrides QAbstractItemView::reset().
     *
     */
    virtual void reset() override;

    /** \brief Exports all the model values to a file in disk in a comma separated values format.
     * \param[in] fileName, QFileInfo object.
     *
     */
    bool exportToCSV(const QFileInfo &filename);

    /** \brief Exports all the model values to a file in disk in Microsoft Excel format.
     * \param[in] fileName, file name.
     *
     */
    bool exportToXLS(const QString &filename);

  public slots:
    /** \brief Updates the selection in the widget.
     * \param[in] selection, selection as a list of segmentation adapter raw pointers.
     *
     */
    void updateSelection(SegmentationAdapterList selection);

  protected slots:
		/** \brief Perform actions when the user double-click a cell in the view.
		 * \param[in] index, model index that has been double-clicked.
		 *
		 */
    void indexDoubleClicked(QModelIndex index);

    /** \brief Updates the data of the specified index.
     * \param[in] index, model index to be updated.
     *
     */
    void updateRepresentation(const QModelIndex &index);

    /** \brief Updates the selection given the selected items and the unselected ones.
     * \param[in] selected, QItemSelection object.
     * \param[in] deselected, QItemSelection object.
     *
     */
    void updateSelection(QItemSelection selected, QItemSelection deselected);

    /** \brief Perform operations after some rows of the model have been removed.
     * \param[in] parent, model index parent of the removed items.
     * \param[in] start, row start value.
     * \param[in] end, row end value.
     *
     * Closes a tab if all elements from that tab have been removed.
     *
     */
    void rowsRemoved(const QModelIndex &parent, int start, int end);

    /** \brief Shows the export information dialog.
     *
     */
    virtual void exportInformation();

    /** \brief Updates the Export button when all the data has been computed.
     *
     */
    void updateExportStatus();

  private:
    /** \brief Returns true if the segmentation should be shown on the report.
     * \param[in] segmentation, segmentation adapter raw pointer of the segmentation to check.
     *
     */
    bool acceptSegmentation(const SegmentationAdapterPtr segmentation);

    /** \brief Creates a tab in the report with the specified name.
     * \param[in] category, name of the tab.
     *
     */
    virtual void createCategoryEntry(const QString &category);

    /** \brief Returns the source model index from the specified view model index.
     * \param[in] index, QModelIndex to translate.
     *
     */
    QModelIndex mapToSource(const QModelIndex &index);

    /** \brief Returns the view model index from the specified source model index.
     * \param[in] index, QModelIndex to translate.
     *
     */
    QModelIndex mapFromSource(const QModelIndex &index, QSortFilterProxyModel *sortFilter);

    /** \brief Removes all tabs and buttons of the report.
     *
     */
    void removeTabsAndWidgets();

    /** \brief Returns the path to store/read the information of the report in the temporal storage.
     * \param[in] file, optional file name.
     *
     */
    static QString extraPath(const QString &file = QString())
    { return "Extra/RawInformation/" + file; }

  protected:
    ModelAdapterSPtr m_model;
    ModelFactorySPtr m_factory;
    ViewManagerSPtr  m_viewManager;

    SegmentationAdapterList m_filter;
    QTabWidget  *m_tabs;
    QPushButton *m_exportButton;

    bool m_multiSelection;
  };

} // namespace ESPINA

#endif // DATAVIEW_H