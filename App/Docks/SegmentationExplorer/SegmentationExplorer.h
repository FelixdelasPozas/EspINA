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
#include <Support/Widgets/DockWidget.h>

// Qt
#include <ui_SegmentationExplorer.h>
#include <QStringListModel>

class QUndoStack;

namespace ESPINA
{
  class SegmentationInspector;

  class SegmentationExplorer
  : public DockWidget
  , public SelectableView
  {
    Q_OBJECT
    class GUI;
  public:
    class Layout;

  public:
    /** \brief SegmentationExplorer class constructor.
     * \param[in] model, model adapter smart pointer.
     * \param[in] factory, factory smart pointer.
     * \param[in] viewManager, view manager smart pointer.
     * \param[in] undoStack, QUndoStack object raw pointer.
     * \param[in] parent, parent widget raw pointer.
     */
    explicit SegmentationExplorer(ModelAdapterSPtr model,
                                  ModelFactorySPtr factory,
                                  ViewManagerSPtr  viewManager,
                                  QUndoStack      *undoStack,
                                  QWidget         *parent = nullptr);

    /** \brief SegmentationExplorer class virtual destructor.
     *
     */
    virtual ~SegmentationExplorer();

    /** \brief Overrides SelectableView::updateRepresentations(ChannelAdapterList).
     *
     */
    virtual void updateRepresentations(ChannelAdapterList list) override
    {}

    /** \brief Overrides SelectableView::updateRepresentations(SegmentationAdapterList).
     *
     */
    virtual void updateRepresentations(SegmentationAdapterList list) override
    {}

    /** \brief Overrides SelectableView::updateRepresentations().
     *
     */
    virtual void updateRepresentations() override
    {}

  public slots:
    /** \brief Overrides DockWidget::reset().
     *
     */
    virtual void reset() override;

  protected:
		/** \brief Overrides SelectableView::onSelectionSet(SelectionSPtr).
		 *
		 */
    virtual void onSelectionSet(SelectionSPtr selection);

    /** \brief Adds a layout to the view.
     * \param[in] id, string that specifies the layout.
     * \param[in] proxy, SegmentationExplorer::Layout raw pointer.
     *
     */
    void addLayout(const QString id, Layout *proxy);

    /** \brief Overrides QObject::eventFilter(QObject*, QEvent*).
     *
     */
    virtual bool eventFilter(QObject *sender, QEvent* e);

    /** \brief Updates segmentation explorer gui depending on selected indexes.
     *
     */
    void updateGUI(const QModelIndexList &selectedIndexes);

  protected slots:
		/** \brief Changes the layout of the view.
		 * \param[in] index, index of the new layout in the layout list.
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
     * \param[in] index, const QModelIndex reference of the selected item.
     *
     */
    void focusOnSegmentation(const QModelIndex &index);

    /** \brief Updates the GUI of the view and the other views based on the selected items.
     * \param[in] selected, QItemSelection object with the selected items (unused).
     * \param[in] deselectt, QItemSelection object with the deselected items (unused).
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

    /** \brief Updates the render views.
     *
     */
    void onItemModified();

  protected:
    ModelAdapterSPtr m_baseModel;
    ViewManagerSPtr  m_viewManager;
    QUndoStack      *m_undoStack;
    GUI             *m_gui;
    QStringList      m_layoutNames;
    QStringListModel m_layoutModel;
    QList<Layout *>  m_layouts;
    Layout          *m_layout;
  };

} // namespace ESPINA

#endif // ESPINA_SEGMENTATION_EXPLORER_H
