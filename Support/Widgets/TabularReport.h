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
#include <GUI/Model/ModelAdapter.h>
#include <Support/Context.h>

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
     * \param[in] context of ESPINA
     * \param[in] parent widget
     * \param[in] flags window flags.
     *
     */
    explicit TabularReport(const Support::Context &context,
                           QWidget                *parent = nullptr,
                           Qt::WindowFlags         flags = Qt::WindowFlags{Qt::WindowNoState});

    /** \brief TabularReport class virtual destructor.
     *
     */
    virtual ~TabularReport();

    virtual int horizontalOffset() const
    { return 0;}

    virtual QModelIndex indexAt(const QPoint &point) const
    { return QModelIndex(); }

    virtual bool isIndexHidden(const QModelIndex &index) const
    { return false; }

    virtual QModelIndex moveCursor(CursorAction cursorAction, Qt::KeyboardModifiers modifiers)
    { return QModelIndex(); }

    virtual void scrollTo(const QModelIndex &index, ScrollHint hint = EnsureVisible)
    {}

    virtual void setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags command)
    {}

    virtual int verticalOffset() const
    { return 0; }

    virtual QRect visualRect(const QModelIndex &index) const
    { return QRect(); }

    virtual QRegion visualRegionForSelection(const QItemSelection &selection) const
    {return QRegion();}

    /** \brief Sets the model to be used.
     * \param[in] model model adapter smart pointer.
     */
    virtual void setModel(ModelAdapterSPtr model);

    /** \brief Sets the filter of the model.
     * \param[in] segmentations list of segmentation adapters to be shown.
     *
     * Only display segmentations' information. If segmentations is empty, then
     * all segmentations' information is displayed.
     *
     */
    virtual void setFilter(SegmentationAdapterList segmentations);

  protected:
    virtual void dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight) override;

    virtual void rowsInserted(const QModelIndex &parent, int start, int end) override;

    virtual bool event(QEvent *event) override;

    virtual void reset() override;

    /** \brief Exports all the model values to a file in disk in a comma separated values format.
     * \param[in] fileName QFileInfo object.
     *
     */
    bool exportToCSV(const QFileInfo &filename);

    /** \brief Exports all the model values to a file in disk in Microsoft Excel format.
     * \param[in] fileName file name.
     *
     */
    bool exportToXLS(const QString &filename);

  public slots:
    /** \brief Updates the selection in the widget.
     * \param[in] selection selection as a list of segmentation adapter raw pointers.
     *
     */
    void updateSelection(SegmentationAdapterList selection);

  protected slots:
    /** \brief Perform actions when the user double-click a cell in the view.
     * \param[in] index model index that has been double-clicked.
     *
     */
    void indexDoubleClicked(QModelIndex index);

    /** \brief Updates the data of the specified index.
     * \param[in] index model index to be updated.
     *
     */
    void updateRepresentation(const QModelIndex &index);

    /** \brief Updates the selection given the selected items and the unselected ones.
     * \param[in] selected QItemSelection object.
     * \param[in] deselected QItemSelection object.
     *
     */
    void updateSelection(QItemSelection selected, QItemSelection deselected);

    /** \brief Perform operations after some rows of the model have been removed.
     * \param[in] parent model index parent of the removed items.
     * \param[in] start row start value.
     * \param[in] end row end value.
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
     * \param[in] segmentation segmentation adapter raw pointer of the segmentation to check.
     *
     */
    bool acceptSegmentation(const SegmentationAdapterPtr segmentation);

    /** \brief Creates a tab in the report with the specified name.
     * \param[in] category name of the tab.
     *
     */
    virtual void createCategoryEntry(const QString &category);

    /** \brief Returns the source model index from the specified view model index.
     * \param[in] index QModelIndex to translate.
     *
     */
    QModelIndex mapToSource(const QModelIndex &index);

    /** \brief Returns the view model index from the specified source model index.
     * \param[in] index QModelIndex to translate.
     *
     */
    QModelIndex mapFromSource(const QModelIndex &index, QSortFilterProxyModel *sortFilter);

    /** \brief Removes all tabs and buttons of the report.
     *
     */
    void removeTabsAndWidgets();

    inline ModelFactorySPtr factory() const;

    /** \brief Returns the path to store/read the information of the report in the temporal storage.
     * \param[in] file optional file name.
     *
     */
    static QString extraPath(const QString &file = QString())
    { return "Extra/RawInformation/" + file; }

  protected:
    const Support::Context &m_context;
    ModelAdapterSPtr m_model;

    SegmentationAdapterList m_filter;
    QTabWidget  *m_tabs;
    QPushButton *m_exportButton;

    bool m_multiSelection;
  };

} // namespace ESPINA

#endif // DATAVIEW_H
