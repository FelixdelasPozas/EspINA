/*
    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>

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

#ifndef ESPINA_SEGMENTATION_EXPLORER_H
#define ESPINA_SEGMENTATION_EXPLORER_H

// ESPINA
#include <Support/Widgets/Panel.h>
#include <Support/Context.h>

// Qt
#include <ui_SegmentationExplorer.h>
#include <GUI/View/SelectableView.h>
#include <Support/Factory/FilterRefinerFactory.h>
#include <QStringListModel>

class QUndoStack;

namespace ESPINA
{
  class SegmentationInspector;

  class SegmentationExplorer
  : public Panel
  , public SelectableView
  {
    Q_OBJECT
    class GUI;
  public:
    class Layout;

  public:
    /** \brief SegmentationExplorer class constructor.
     */
    explicit SegmentationExplorer(Support::FilterRefinerFactory &filterRefiners,
                                  Support::Context               &context);

    /** \brief SegmentationExplorer class virtual destructor.
     *
     */
    virtual ~SegmentationExplorer();

  public slots:
    virtual void reset() override;

  protected:
    /** \brief Adds a layout to the view.
     * \param[in] id string that specifies the layout.
     * \param[in] proxy SegmentationExplorer::Layout raw pointer.
     *
     */
    void addLayout(const QString &id, Layout *proxy);

    virtual bool eventFilter(QObject *sender, QEvent* e) override;

    /** \brief Updates segmentation explorer gui depending on selected indexes.
     *
     */
    void updateGUI(const QModelIndexList &selectedIndexes);

  private slots:
    /** \brief Changes the layout of the view.
     * \param[in] index index of the new layout in the layout list.
     *
     */
    void changeLayout(int index);

    /** \brief Deletes the selected items in the current layout.
     *
     */
    void deleteSelectedItems();

    /** \brief Shows the information for the selected items in the current layout.
     *
     */
    void showSelectedItemsInformation();

    /** \brief Centers the views on the selected segmentation.
     * \param[in] index const QModelIndex reference of the selected item.
     *
     */
    void focusOnSegmentation(const QModelIndex &index);

    /** \brief Updates the GUI of the view and the other views based on the selected items.
     * \param[in] selected QItemSelection object with the selected items (unused).
     * \param[in] deselectt QItemSelection object with the deselected items (unused).
     *
     */
    void onModelSelectionChanged(QItemSelection selected, QItemSelection deselected);

    /** \brief Updates the state of search button of GUI search filter and updates the filter.
     *
     */
    void updateSearchFilter();

    /** \brief Updates the GUI state with the new selection.
     *
     */
    void onSelectionChanged();

    /** \brief Updates the search box with the selected tag.
     *
     */
    void onTagSelected(const QString &tag);

    /** \brief Selects the next segmentation in the tree view and focuses the views on it's centroid.
     *
     */
    void incrementSelection();

    /** \brief Selects the previous segmentation in the tree view and focuses the views on it's centroid.
     *
     */
    void decrementSelection();

  private:

    /** \brief Enum class to specify the movement direction in the nextIndex() private method.
     *
     */
    enum class direction: char { FORWARD = 0, BACKWARD };

    /** \brief Returns the next QModelIndex belonging to a segmentation in regard to the given one in the given direction in the tree view model.
     * \param[in] index previous QModelIndex object.
     * \param[in] dir   movement direction.
     *
     */
    QModelIndex nextIndex(const QModelIndex &index, direction dir);

    /** \brief Creates shortcuts to go forwards/backwards on segmentation selection.
     *
     */
    void createShortcuts();

    /** \brief Returns the list of selected indexes in the selection model of the tree view.
     *
     */
    QModelIndexList selectedIndexes() const;

    /** \brief Updates the tags in the GUI with the ones of the selected indexes.
     * \param[in] selectedIndexes QModelIndex list.
     *
     */
    void updateTags(const QModelIndexList &selectedIndexes);

  protected:
    GUI             *m_gui;
    QStringList      m_layoutNames;
    QStringListModel m_layoutModel;
    QList<Layout *>  m_layouts;
    Layout          *m_layout;
  };

} // namespace ESPINA

#endif // ESPINA_SEGMENTATION_EXPLORER_H
