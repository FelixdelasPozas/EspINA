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

#ifndef ESPINA_CLASSIFICATION_PROXY_H
#define ESPINA_CLASSIFICATION_PROXY_H

#include <QAbstractProxyModel>
#include <GUI/Model/ModelAdapter.h>

namespace ESPINA
{
  /** \class ClassificationProxy.
   * \brief Group Segmentations by Category
   *
   */
  class EspinaGUI_EXPORT ClassificationProxy
  : public QAbstractProxyModel
  {
    Q_OBJECT
  public:
    /** brief ClassificationProxy class constructor.
     * \param[in] sourceModel, model adapter smart pointer.
     * \param[in] parent, raw pointer of the parent of this object.
     *
     */
    ClassificationProxy(ModelAdapterSPtr sourceModel, QObject *parent = nullptr);

    /** brief ClassificationProxy class virtual destructor.
     *
     */
    virtual ~ClassificationProxy();

    /** brief Sets the model for this proxy.
     *
     */
    virtual void setSourceModel(ModelAdapterSPtr sourceModel);

    /** brief Overrides QAbstractProxyModel::data().
     *
     */
    virtual QVariant data(const QModelIndex& proxyIndex, int role = Qt::DisplayRole) const override;

    /** brief Overrides QAbstractProxyModel::setData().
     *
     */
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

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

    /** brief Overrides QAbstractProxyModel::flags().
     *
     * Drag & Drop support
     *
     */
    virtual Qt::ItemFlags flags(const QModelIndex& index) const override;

    /** brief Overrides QAbstractProxyModel::supportedDropActions().
     *
		 * Drag & Drop support
     *
     */
    virtual Qt::DropActions supportedDropActions() const {return Qt::MoveAction;}

    /** brief Overrides QAbstractProxyModel::dropMimeData().
     *
     * Drag & Drop support
     *
     */
    virtual bool dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent);

    /** brief Returns the number of segmentation contained in a category.
     * \param[in] categoryIndex, model index of the category.
     * \param[in] recursive, true to count also segmentations in sub-categories, false otherwise.
     *
     */
    int numSegmentations(QModelIndex categoryIndex, bool recursive = false) const;

    /** brief Returns the number of segmentation contained in a category.
     * \param[in] category, category adapter raw pointer.
     * \param[in] recursive, true to count also segmentations in sub-categories, false otherwise.
     *
     */
    int numSegmentations(CategoryAdapterPtr category, bool recursive = false) const;

    /** brief Returns the number of sub-categories contained in a category.
     * \param[in] categoryIndex, model index of the category.
     *
     */
    int numSubCategories(QModelIndex categoryIndex) const;

    /** brief Returns the number of sub-categories contained in a category.
     * \param[in] category, category adapter raw pointer.
     *
     */
    int numSubCategories(CategoryAdapterPtr category) const;

    /** brief Returns the list of indexes of the model for the segmentations contained in a category.
     * \param[in] categoryIndex, model index of the category.
     * \param[in] recursive, true to count also segmentations in sub-categories, false otherwise.
     *
     */
    QModelIndexList segmentations(QModelIndex categoryIndex, bool recursive = false) const;

    /** brief Returns the list of indexes of the model for the segmentations contained in a category.
     * \param[in] category, categoryAdapter raw pointer.
     * \param[in] recursive, true to count also segmentations in sub-categories, false otherwise.
		 *
     */
    SegmentationAdapterList segmentations(CategoryAdapterPtr category, bool recursive = false) const;

  protected slots:
		/** brief Inserts rows in the model given the parent in the source model and the interval.
		 * \param[in] sourceParent, model index of the parent item in the source model.
		 * \param[in] start, start value.
		 * \param[in] end, end value.
		 *
		 */
    void sourceRowsInserted(const QModelIndex & sourceParent, int start, int end);

		/** brief Removes rows in the model given the parent in the source model and the interval, before the rows being removed in the source model.
		 * \param[in] sourceParent, model index of the parent item in the source model.
		 * \param[in] start, start value.
		 * \param[in] end, end value.
		 *
		 */
    void sourceRowsAboutToBeRemoved(const QModelIndex & sourceParent, int start, int end);

		/** brief Removes rows in the model given the parent in the source model and the interval.
		 * \param[in] sourceParent, model index of the parent item in the source model.
		 * \param[in] start, start value.
		 * \param[in] end, end value.
		 *
		 */
    void sourceRowsRemoved(const QModelIndex & sourceParent, int start, int end);

		/** brief Moves rows in the model given the parent in the source model and the interval, before the rows being moved in the source model.
		 * \param[in] sourceParent, model index of the parent item in the source model.
		 * \param[in] sourceStart, start value.
		 * \param[in] sourceEnd, end value.
		 * \param[in] destinationParent, model index of the destination in the source model.
		 * \param[in] destinationRow, destination row.
		 *
		 */
    void sourceRowsAboutToBeMoved(const QModelIndex &sourceParent, int sourceStart, int sourceEnd, const QModelIndex & destinationParent, int destinationRow);

		/** brief Moves rows in the model given the parent in the source model and the interval.
		 * \param[in] sourceParent, model index of the parent item in the source model.
		 * \param[in] sourceStart, start value.
		 * \param[in] sourceEnd, end value.
		 * \param[in] destinationParent, model index of the destination in the source model.
		 * \param[in] destinationRow, destination row.
		 *
		 */
    void sourceRowsMoved(const QModelIndex &sourceParent, int sourceStart, int sourceEnd, const QModelIndex & destinationParent, int destinationRow);

    /** brief Updates the data in the model when it changes in the source model.
     * \param[in] sourceTopLeft, source model index top left.
     * \param[in] sourceBottomRight, source model index bottom right.
     */
    void sourceDataChanged(const QModelIndex &sourceTopLeft, const QModelIndex &sourceBottomRight);

    /** brief Resets the model of the proxy.
     *
     */
    void sourceModelReset();

  signals:
    void categoriesDropped(CategoryAdapterList sources, CategoryAdapterPtr parent);
    void segmentationsDropped(SegmentationAdapterList sources, CategoryAdapterPtr category);

  protected:
    /** brief Retuns the list of QModelIndex the the selected interval in the parameter.
     * \param[in] topLeft, top left index of the selection.
     * \param[in] bottomRight, bottom right index of the selection.
     * \param[out] result, list of QModelIndex of the selection.
     *
     */
    bool indices(const QModelIndex& topLeft, const QModelIndex& bottomRight, QModelIndexList& result);

    /** brief Returns the segmentation adapter with given the tooltip.
     * \param[in] tooltip, text tooltip.
     *
     */
    SegmentationAdapterPtr findSegmentation(QString tooltip);

    /** brief Returns the list of source model indexes that correspond to the selected interval in the proxy model.
     * \param[in] parent, model index in the proxy model.
     * \param[in] start, interval start.
     * \param[in] end, interval end.
     *
     */
    QModelIndexList sourceIndices(const QModelIndex& parent, int start, int end) const;

    /** brief Returns the list of proxy model indexes that correspond to the selected interval in the source model.
     * \param[in] parent, model index in the source model.
     * \param[in] start, interval start.
     * \param[in] end, interval end.
     *
     */
    QModelIndexList proxyIndices(const QModelIndex& parent, int start, int end) const;

    /** brief Adds a category to the model.
     * \param[in] category, category adapter raw pointer.
     *
     */
    void addCategory(CategoryAdapterPtr category);

    /** brief Removes a category from the model.
     * \param[in] category, category adapter raw pointer.
     *
     */
    void removeCategory(CategoryAdapterPtr category);

    /** brief Returns the index of the category in the proxy model.
     * \param[in] category, category adapter raw pointer.
     */
    QModelIndex categoryIndex(CategoryAdapterPtr category) const;

  private:
    /** brief Creates and returns a category adapter raw pointer from an analogous category from the source model.
     * \param[in] sourceCategory, source category adapter raw pointer.
     *
     */
    CategoryAdapterPtr createProxyCategory(CategoryAdapterPtr sourceCategory);

    /** brief Returns the proxy index for the specified category, row and column.
     * \param[in] row.
     * \param[in] column.
     * \param[in] category, source category adapter raw pointer.
     *
     */
    QModelIndex createSourceCategoryIndex(int row, int column, CategoryAdapterPtr category) const;

    /** brief Returns the category adapter raw pointer from the proxy that corresponds to the specified source model category adapter.
     * \param[in] sourceCategory, raw pointer of a category from the source model.
     *
     */
    CategoryAdapterPtr toProxyPtr(CategoryAdapterPtr sourceCategory) const;

    /** brief Returns the category adapter smart pointer from the proxy that corresponds to the specified source model category adapter.
     * \param[in] sourceCategory, raw pointer of a category from the source model.
     *
     */
    CategoryAdapterSPtr toProxySPtr(CategoryAdapterPtr sourceCategory) const;

    /** brief Returns the category adapter raw pointer from the source that corresponds to the specified proxy model category adapter.
     * \param[in] proxyCategory, raw pointer of a category from the proxy model.
     *
     */
    CategoryAdapterPtr toSourcePtr(CategoryAdapterPtr proxyCategory) const;

    /** brief Returns the category adapter smart pointer from the source that corresponds to the specified proxy model category adapter.
     * \param[in] proxyCategory, raw pointer of a category from the proxy model.
     *
     */
    CategoryAdapterSPtr toSourceSPtr(CategoryAdapterPtr proxyCategory) const;

  private:
    ModelAdapterSPtr m_model;
    // Keep a reference to the categories which belong to the classification root
    CategoryAdapterList m_rootCategories;

    // We need to rely on our own row count for each item in the proxy's model
    // If we rely on the source's model, there are some inconsistencies during
    // rows insertion/deletion
    mutable QMap<CategoryAdapterPtr, CategoryAdapterPtr> m_sourceCategory;
    mutable ClassificationAdapterSPtr                    m_classification;
    mutable QMap<CategoryAdapterPtr, int            >    m_numCategories;
    mutable QMap<CategoryAdapterPtr, ItemAdapterList>    m_categorySegmentations;
    mutable QMap<CategoryAdapterPtr, Qt::CheckState >    m_categoryVisibility;
  };

} // namespace ESPINA

#endif // ESPINA_CLASSIFICATION_PROXY_H
