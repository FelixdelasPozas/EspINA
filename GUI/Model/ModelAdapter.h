/*

    Copyright (C) 2014  Jorge Peña Pastor<jpena@cesvima.upm.es>

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

#ifndef ESPINA_MODEL_ADAPTER_H
#define ESPINA_MODEL_ADAPTER_H

#include "GUI/EspinaGUI_Export.h"

// ESPINA
#include "GUI/Model/SampleAdapter.h"
#include "GUI/Model/ChannelAdapter.h"
#include "GUI/Model/ClassificationAdapter.h"
#include "GUI/Model/SegmentationAdapter.h"
#include <GUI/ModelFactory.h>

// Qt
#include <QAbstractItemModel>

namespace ESPINA
{
  struct Relation
  {
    ItemAdapterSPtr ancestor;
    ItemAdapterSPtr succesor;
    RelationName    relation;
  };
  typedef QList<Relation> RelationList;

  class Analysis;
  using AnalysisSPtr = std::shared_ptr<Analysis>;

  /** \class ModelAdapter
  *   \brief Adapt analysis to Qt Model framework
  * Model elements are arranged in the following way:
  * QModelIndex() (invalid index/model root index)
  * - ClassificationRoot
  *   - Category 1
  *     - Sub-Catagory 1-1
  *     - ...
  *   - Category 2
  *     - ...
  *   - ...
  * - SampleAdapterRoot
  *   - SampleAdapter1
  *   - ...
  * - ChannelRoot
  *   - Channel1
  *   - ...
  * - SegmentationRoot
  *   - Segmentation1
  *   - ...
  * - FilterRoot
  *   - Filter1
  *   - ...
  */
  class EspinaGUI_EXPORT ModelAdapter
  : public QAbstractItemModel
  {
    Q_OBJECT
  public:
    struct Existing_Item_Exception{};
    struct Existing_Relation_Exception{};
    struct Item_Not_Found_Exception {};
    struct Relation_Not_Found_Exception {};

  public:
    /** \brief ModelAdapter class constructor.
     *
     */
    explicit ModelAdapter();

    /** \brief ModelAdapter class destructor.
     *
     */
    virtual ~ModelAdapter();

    /** \brief Sets the analysis this model adapts and a model factory.
     * \param[in] analysis, analysis smart pointer.
     * \param[in] factory, model factory smart pointer.
     *
     */
    void setAnalysis(AnalysisSPtr analysis, ModelFactorySPtr factory);

    /** \brief Shadows QAbstractItemModel::reset().
     *
     */
    void reset();

    /** \brief Sets the model temporal storage.
     * \param[in] storage, temporal storage smart pointer.
     *
     */
    void setStorage(TemporalStorageSPtr storage);

    /** \brief Returns the model's temporal storage.
     *
     */
    TemporalStorageSPtr storage() const;

    /** \brief Implements QAbstractItemModel::data().
     *
     */
    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;

    /** \brief Overrides QAbstractItemModel::setData().
     *
     */
    virtual bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;

    /** \brief Overrides QAbstractItemModel::itemData().
     *
     */
    virtual QMap<int, QVariant> itemData(const QModelIndex &index) const override;

    /** \brief Overrides QAbstractItemModel::flags().
     *
     */
    virtual Qt::ItemFlags flags(const QModelIndex& index) const override;

    /** \brief Implements QAbstractItemModel::columnCount().
     *
     */
    virtual int columnCount(const QModelIndex& parent = QModelIndex()) const;

    /** \brief Implements QAbstractItemModel::rowCount().
     *
     */
    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;

    /** \brief Implements QAbstractItemModel::parent().
     *
     */
    virtual QModelIndex parent(const QModelIndex& child) const;

    /** \brief Implements QAbstractItemModel::index().
     *
     */
    virtual QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const;

    /** \brief Returns the model index of the given item.
     * \param[in] item, item adapter raw pointer.
     *
     */
    QModelIndex index(ItemAdapterPtr item) const;

    /** \brief Returns the model index of the given item.
     * \param[in] item, item adapter smart pointer.
     *
     */
    QModelIndex index(ItemAdapterSPtr item) const;

    /** \brief Returna the model index of the classification root node.
     *
     * Special Nodes of the model to refer different roots.
     *
     */
    QModelIndex classificationRoot() const;

    /** \brief Returns the model index of the given category.
     * \param[in] category, category adapter raw pointer.
     *
     */
    QModelIndex categoryIndex(CategoryAdapterPtr category) const;

    /** \brief Returns the model index of the given category.
     * \param[in] category, category adapter smart pointer.
     *
     */
    QModelIndex categoryIndex(CategoryAdapterSPtr category) const;

    /** \brief Returns the model index of the sample root node.
     *
     * Special Nodes of the model to refer different roots.
     *
     */
    QModelIndex sampleRoot() const;

    /** \brief Returns the model index of the given sample.
     * \param[in] sample, sample adapter raw pointer.
     *
     */
    QModelIndex sampleIndex(SampleAdapterPtr sample) const;

    /** \brief Returns the model index of the given sample.
     * \param[in] sample, sample adapter smart pointer.
     *
     */
    QModelIndex sampleIndex(SampleAdapterSPtr sample) const;

    /** \brief Returns the model index of the channels root node.
     *
     * Special Nodes of the model to refer different roots.
     *
     */
    QModelIndex channelRoot() const;

    /** \brief Returns the model index of the given channel.
     * \param[in] channel, channel adapter raw pointer.
     *
     */
    QModelIndex channelIndex(ChannelAdapterPtr  channel) const;

    /** \brief Returns the model index of the given channel.
     * \param[in] channel, channel adapter smart pointer.
     *
     */
    QModelIndex channelIndex(ChannelAdapterSPtr channel) const;

    /** \brief Returns the model index of the segmentation root node.
     *
     * Special Nodes of the model to refer different roots.
     *
     */
    QModelIndex segmentationRoot() const;

    /** \brief Returns the model index of the given segmentation.
     * \param[in] segmentation, segmentation adapter raw pointer.
     *
     */
    QModelIndex segmentationIndex(SegmentationAdapterPtr  segmentation) const;

    /** \brief Returns the model index of the given segmentation.
     * \param[in] segmentation, segmentation adapter smart pointer.
     *
     */
    QModelIndex segmentationIndex(SegmentationAdapterSPtr segmentation) const;

    /** \brief Sets the classification of the model.
     * \param[in] classification, classification adapter smart pointer.
     *
     */
    void setClassification(ClassificationAdapterSPtr classification);

    /** \brief Returns the classification adapter smart pointer of the model.
     *
     */
    const ClassificationAdapterSPtr classification() const;

    /** \brief Creates the root node of the categories.
     * \param[in] name, name of the root category.
     *
     * Special Nodes of the model to refer different roots.
     *
     */
    CategoryAdapterSPtr createRootCategory(const QString& name);

     /** \brief Creates a category and returns it's category adapter smart pointer.
      * \param[in] name, name of the category.
      * \param[in] parent, raw pointer category adapter of the parent of the new category.
     *
     */
    CategoryAdapterSPtr createCategory(const QString& name, CategoryAdapterPtr  parent);

    /** \brief Creates a category and returns it's category adapter smart pointer.
     * \param[in] name, name of the category.
     * \param[in] parent, smart pointer category adapter of the parent of the new category.
    *
    */
    CategoryAdapterSPtr createCategory(const QString& name, CategoryAdapterSPtr parent);

    /** \brief Adds a category to the model.
     * \param[in] category, smart pointer of the category adapter to add.
     * \param[in] parent, smart pointer of the category adapter parent of the added category.
     *
     */
    void addCategory(CategoryAdapterSPtr category, CategoryAdapterSPtr parent);

    /** \brief Removes a category to the model.
     * \param[in] category, smart pointer of the category adapter to remove.
     * \param[in] parent, smart pointer of the category adapter parent of the added category.
     *
     */
    void removeCategory(CategoryAdapterSPtr category, CategoryAdapterSPtr parent);

    /** \brief Removes a category whose parent is the root node.
     * \param[in] category, smart pointer of the category adapter to remove.
     *
     */
    void removeRootCategory(CategoryAdapterSPtr category);

    /** \brief Changes the parent of an existing category.
     * \param[in] category, smart pointer of the category to change parent.
     * \param[in] parent, smart pointer of the category that is the new parent.
     *
     * TODO 2013-10-21: Throw exception if they don't belong to the same classification
     *
     */
    void reparentCategory(CategoryAdapterSPtr category, CategoryAdapterSPtr parent);

    /** \brief Adds a sample to the model.
     * \param[in] sample, smart pointer of the sample adapter to add.
     *
     */
    void add(SampleAdapterSPtr sample);

    /** \brief Adds a list of samples to the model.
     * \param[in] samples, list of smart pointers of the sample adapters to add.
     *
     */
    void add(SampleAdapterSList samples);

    /** \brief Adds a channel to the model.
     * \param[in] channel, smart pointer of the channel adapter to add.
     *
     */
    void add(ChannelAdapterSPtr channel);

    /** \brief Adds a list of channels to the model.
     * \param[in] channels, list of smart pointers of the channels adapters to add.
     *
     */
    void add(ChannelAdapterSList channels);

    /** \brief Adds a segmentation to the model.
     * \param[in] segmentation, smart pointer of the segmentation adapter to add.
     *
     */
    void add(SegmentationAdapterSPtr segmentation);

    /** \brief Adds a list of segmentations to the model.
     * \param[in] segmentations, list of smart pointers of the segmentations adapters to add.
     *
     */
    void add(SegmentationAdapterSList segmentations);

    /** \brief Removes a sample from the model.
     * \param[in] sample, smart pointer of the sample adapter to remove.
     *
     */
    void remove(SampleAdapterSPtr sample);

    /** \brief Removes a list of samples from the model.
     * \param[in] samples, list of smart pointers of the sample adapters to remove.
     *
     */
    void remove(SampleAdapterSList samples);

    /** \brief Removes a channel from the model.
     * \param[in] channel, smart pointer of the channel adapter to remove.
     *
     */
    void remove(ChannelAdapterSPtr channel);

    /** \brief Removes a list of channels from the model.
     * \param[in] channels, list of smart pointers of the channel adapters to remove.
     *
     */
    void remove(ChannelAdapterSList channels);

    /** \brief Removes a segmentation from the model.
     * \param[in] segmentation, smart pointer of the segmentation adapter to remove.
     *
     */
    void remove(SegmentationAdapterSPtr segmentation);

    /** \brief Removes a list of segmentations from the model.
     * \param[in] segmentations, list of smart pointers of the segmentation adapters to remove.
     *
     */
    void remove(SegmentationAdapterSList segmentations);

    /** \brief Returns the list of sample adapters in the model.
     *
     */
    SampleAdapterSList samples() const
    { return m_samples; }

    /** \brief Returns the list of channel adapters in the model.
     *
     */
    ChannelAdapterSList channels() const
    { return m_channels; }

    /** \brief Returns the list of segmentation adapters in the model.
     *
     */
    SegmentationAdapterSList segmentations() const
    { return m_segmentations; }

    /** \brief Sets the category of a segmentation.
     * \param[in] segmentation, smart pointer of the segmentation adapter to change.
     * \param[in] category, smart pointer of the new category adapter.
     *
     */
    void setSegmentationCategory(SegmentationAdapterSPtr segmentation,
                                 CategoryAdapterSPtr     category);


    /** \brief Adds a relation between two item adapters in the model.
     * \param[in] ancestor, item adapter smart pointer origin of the relation.
     * \param[in] succesor, item adapter smart pointer destination of the relation.
     * \param[in] relation, text string that specifies the relation.
     *
     */
    void addRelation(ItemAdapterSPtr     ancestor,
                     ItemAdapterSPtr     succesor,
                     const RelationName& relation);

    /** \brief Adds a relation to the model.
     * \param[in] relation, valid relation object.
     *
     */
    void addRelation(const Relation& relation);

    /** \brief Adds a list of relations to the model.
     * \param[in] relationList, list of Relation objects.
     *
     */
    void addRelations(const RelationList &relations);


    /** \brief Removes a relation between two item adapters from the model.
     * \param[in] ancestor, item adapter smart pointer origin of the relation.
     * \param[in] succesor, item adapter smart pointer destination of the relation.
     * \param[in] relation, text string that specifies the relation.
     *
     */
    void deleteRelation(ItemAdapterSPtr     ancestor,
                        ItemAdapterSPtr     succesor,
                        const RelationName& relation);

    /** \brief Deletes a relation from the model.
     * \param[in] relation, valid relation object.
     *
     */
    void deleteRelation(const Relation& relation);

    /** \brief Deletes a list of relations to the model.
     * \param[in] relationList, list of Relation objects.
     *
     */
    void deleteRelations(const RelationList &relations);

    /** \brief Returns the list of item adapters related to the specified one.
     * \param[in] item, item adapter raw pointer.
     * \param[in] type, type of the relation.
     * \param[in] filter, relations filter.
     *
     */
    ItemAdapterSList relatedItems(ItemAdapterPtr item, RelationType type, const RelationName& filter = QString());

    /** \brief Returns the list of relations that an item have that comply with the specified type and filter.
     * \param[in] item, item adapter raw pointer.
     * \param[in] type, relation type.
     * \param[in] filter, relations filter.
     *
     */
    RelationList relations(ItemAdapterPtr item, RelationType type, const RelationName& filter = QString());

    /** \brief Emits the added segmentation signal for the given segmentation.
     * \param[in] segmentation, segmentation adapter smart pointer.
     *
     * Used by undo commands to signal finished operations.
     *
     */
    void emitSegmentationsAdded(SegmentationAdapterSPtr segmentation);

    /** \brief Emits the added segmentation signal for the given list of segmentation adapters.
     * \param[in] segmentations, list of segmentation adapters.
     *
     * Used by undo commands to signal finished operations.
     *
     */
    void emitSegmentationsAdded(SegmentationAdapterSList segmentations);

    /** \brief Emits the added channel signal for the given list of channel adapters.
     * \param[in] channels, list of channel adapters.
     *
     * Used by undo commands to signal finished operations.
     *
     */
    void emitChannelAdded(ChannelAdapterSList channels);

    //---------------------------------------------------------------------------
    /************************** SmartPointer API *******************************/
    //---------------------------------------------------------------------------

    /** \brief Returns the item adapter smart pointer of the item specified by it persistent smart pointer.
     * \param[in] item, persistent smart pointer.
     *
     */
    ItemAdapterSPtr find(PersistentSPtr item);

    /** \brief Returns the smart pointer of a category adapter given its raw pointer.
     * \param[in] category, category adapter raw pointer.
     *
     */
    CategoryAdapterSPtr smartPointer(CategoryAdapterPtr category);

    /** \brief Returns the smart pointer of a sample adapter given its raw pointer.
     * \param[in] sample, sample adapter raw pointer.
     *
     */
    virtual SampleAdapterSPtr smartPointer(SampleAdapterPtr sample);

    /** \brief Returns the smart pointer of a channel adapter given its raw pointer.
     * \param[in] channel, channel adapter raw pointer.
     *
     */
    virtual ChannelAdapterSPtr smartPointer(ChannelAdapterPtr channel);

    /** \brief Returns the smart pointer of a segmentation adapter given its raw pointer.
     * \param[in] segmentation, segmentation adapter raw pointer.
     *
     */
    virtual SegmentationAdapterSPtr smartPointer(SegmentationAdapterPtr segmentation);

  signals:
    void classificationAdded  (ClassificationAdapterSPtr classification);
    void classificationRemoved(ClassificationAdapterSPtr classification);

    void sampleAdded  (SampleAdapterSPtr samples);
    void sampleRemoved(SampleAdapterSPtr samples);

    void channelAdded  (ChannelAdapterSPtr channel);
    void channelRemoved(ChannelAdapterSPtr channel);

    void segmentationsAdded  (SegmentationAdapterSList segmentations);
    void segmentationsRemoved(SegmentationAdapterSList segmentations);

  private slots:
		/** \brief Perform operations when a item has been modified.
		 * \param[in] item, item adapter smart pointer.
		 *
		 */
    void itemModified(ItemAdapterSPtr item);

  private:
    /** \brief Adds an initialized sample adapter to the model, and its correspondent sample to the analysis.
     * \param[in] sample, sample adapter smart pointer.
     *
     */
    void addImplementation(SampleAdapterSPtr sample) throw(Existing_Item_Exception);

    /** \brief Adds an initialized channel adapter to the model, and its correspondent channel to the analysis.
     * \param[in] channel, channel adapter smart pointer.
     *
     */
    void addImplementation(ChannelAdapterSPtr channel) throw(Existing_Item_Exception);

    /** \brief Adds an initialized segmentation adapter to the model, and its correspondent segmentation to the analysis.
     * \param[in] segmentation, segmentation adapter smart pointer.
     *
     */
    void addImplementation(SegmentationAdapterSPtr segmentation) throw(Existing_Item_Exception);

    /** \brief Removes a sample adapter from the model and its adapted sample from the analysis.
     * \param[in] sample, sample adapter smart pointer.
     *
     */
    void removeImplementation(SampleAdapterSPtr sample);

    /** \brief Removes a channel adapter from the model and its adapted channel from the analysis.
     * \param[in] channel, channel adapter smart pointer.
     *
     */
    void removeImplementation(ChannelAdapterSPtr channel);

    /** \brief Removes a segmentation adapter from the model and its adapted segmentation from the analysis.
     * \param[in] segmentation, segmentation adapter smart pointer.
     *
     */
    void removeImplementation(SegmentationAdapterSPtr segmentation);

  private:
    AnalysisSPtr              m_analysis;
    SampleAdapterSList        m_samples;
    ChannelAdapterSList       m_channels;
    SegmentationAdapterSList  m_segmentations;
    ClassificationAdapterSPtr m_classification;
  };

  using ModelAdapterPtr  = ModelAdapter *;
  using ModelAdapterSPtr = std::shared_ptr<ModelAdapter>;

  /** \brief Returns true if the given index is an item adapter.
   * \param[in] index, model index.
   *
   */
  ItemAdapterPtr EspinaGUI_EXPORT itemAdapter(const QModelIndex &index);

  /** \brief Returns true if the given item adpter is a classification item.
   * \param[in] item item adapter raw pointer.
   *
   */
  bool EspinaGUI_EXPORT isClassification(ItemAdapterPtr item);

  /** \brief Returns true if the given item is a category item.
   * \param[in] item item adapter raw pointer.
   *
   */
  bool EspinaGUI_EXPORT isCategory(ItemAdapterPtr item);
} // namespace ESPINA

#endif // ESPINA_MODEL_ADAPTER_H
