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

#ifndef ESPINA_MODEL_ADAPTER_H
#define ESPINA_MODEL_ADAPTER_H

#include "GUI/EspinaGUI_Export.h"

// ESPINA
#include <GUI/Model/SampleAdapter.h>
#include <GUI/Model/ChannelAdapter.h>
#include <GUI/Model/ClassificationAdapter.h>
#include <GUI/Model/SegmentationAdapter.h>
#include <GUI/ModelFactory.h>
#include <GUI/Model/Utils/DBVH.h>
#include <GUI/Model/Utils/SegmentationLocator.h>

// Qt
#include <QAbstractItemModel>
#include <QHash>

namespace ESPINA
{
  class Analysis;
  using AnalysisSPtr = std::shared_ptr<Analysis>;

  struct Relation
  {
    ItemAdapterSPtr ancestor;
    ItemAdapterSPtr successor;
    RelationName    relation;
  };
  using RelationList = QList<Relation>;

  struct Connection
  {
    SegmentationAdapterSPtr item1;
    SegmentationAdapterSPtr item2;
    NmVector3               point;

    /** \brief Connection struct constructor.
     *
     */
    Connection(): item1{nullptr}, item2{nullptr}, point{NmVector3()} {};

    /** \brief Connection struct constructor.
     * \param[in] seg1 first segmentation.
     * \param[in] seg2 second segmentation.
     * \param[in] point connection point.
     *
     */
    Connection(const SegmentationAdapterSPtr seg1, const SegmentationAdapterSPtr seg2, const NmVector3 p)
    : item1{seg1}, item2{seg2}, point{p} {};

    /** \brief Operator== for connections.
     * \param[in] other Connection struct to compare.
     *
     */
    bool operator==(const Connection &other);
  };
  using ConnectionList = QList<Connection>;

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
    private:
      class BatchCommand;
      using BatchCommandSPtr = std::shared_ptr<BatchCommand>;
      using CommandQueue     = QList<BatchCommandSPtr>;
      using CommandQueueList = QList<CommandQueue>;

      struct ItemCommands
      {
        ItemAdapterSPtr Item;
        CommandQueue    Commands;
      };
      using ItemCommandsList = QList<ItemCommands>;

      struct ConsecutiveQueues
      {
        QModelIndex  StartIndex;
        QModelIndex  EndIndex;
        CommandQueue Commands;
      };

      using ConsecutiveQueuesList = QList<ConsecutiveQueues>;

    public:
      /** \brief ModelAdapter class constructor.
       *
       */
      explicit ModelAdapter();

      /** \brief ModelAdapter class destructor.
       *
       */
      virtual ~ModelAdapter()
      {};

      /** \brief Sets the analysis this model adapts and a model factory.
       * \param[in] analysis analysis smart pointer.
       * \param[in] factory model factory smart pointer.
       *
       * NOTE: Doesn't support Batch Mode
       */
      void setAnalysis(AnalysisSPtr analysis, ModelFactorySPtr factory);

      /** \brief Returns true if the model doesn't contains at least
       *         one samples, channel or segmentation
       */
      bool isEmpty() const;

      /** \brief Clears current analysis items and reset views
       *
       * NOTE: Doesn't support Batch Mode
       */
      void clear();

      /** \brief Sets the model temporal storage.
       * \param[in] storage temporal storage smart pointer.
       *
       */
      void setStorage(TemporalStorageSPtr storage);

      /** \brief Returns the model's temporal storage.
       *
       */
      TemporalStorageSPtr storage() const;

      virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

      virtual bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;

      virtual QMap<int, QVariant> itemData(const QModelIndex &index) const override;

      virtual Qt::ItemFlags flags(const QModelIndex& index) const override;

      virtual int columnCount(const QModelIndex& parent = QModelIndex()) const override;

      virtual int rowCount(const QModelIndex& parent = QModelIndex()) const override;

      virtual QModelIndex parent(const QModelIndex& child) const override;

      virtual QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;

      /** \brief Returns the model index of the given item.
       * \param[in] item item adapter raw pointer.
       *
       */
      QModelIndex index(ItemAdapterPtr item) const;

      /** \brief Returns the model index of the given item.
       * \param[in] item item adapter smart pointer.
       *
       */
      QModelIndex index(ItemAdapterSPtr item) const;

      /** \brief Returns the model index of the classification root node.
       *
       * Special Nodes of the model to refer different roots.
       *
       */
      QModelIndex classificationRoot() const;

      /** \brief Returns the model index of the given category.
       * \param[in] category category adapter raw pointer.
       *
       */
      QModelIndex categoryIndex(CategoryAdapterPtr category) const;

      /** \brief Returns the model index of the given category.
       * \param[in] category category adapter smart pointer.
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
       * \param[in] sample sample adapter raw pointer.
       *
       */
      QModelIndex sampleIndex(SampleAdapterPtr sample) const;

      /** \brief Returns the model index of the given sample.
       * \param[in] sample sample adapter smart pointer.
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
       * \param[in] channel channel adapter raw pointer.
       *
       */
      QModelIndex channelIndex(ChannelAdapterPtr  channel) const;

      /** \brief Returns the model index of the given channel.
       * \param[in] channel channel adapter smart pointer.
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
       * \param[in] segmentation segmentation adapter raw pointer.
       *
       */
      QModelIndex segmentationIndex(SegmentationAdapterPtr  segmentation) const;

      /** \brief Returns the model index of the given segmentation.
       * \param[in] segmentation segmentation adapter smart pointer.
       *
       */
      QModelIndex segmentationIndex(SegmentationAdapterSPtr segmentation) const;


      /** \brief Switch the behaviour of the model to group operations until batch mode is finished
       */
      void beginBatchMode();

      /** \brief Switch the behaviour of the model to execute operations when they are invoked
       *
       *   Note: This method increments the model time stamp before executing the queued operations
       */
      void endBatchMode();

      /** \brief Sets the classification of the model.
       * \param[in] classification classification adapter smart pointer.
       *
       * NOTE: Doesn't support Batch Mode
       */
      void setClassification(ClassificationAdapterSPtr classification);

      /** \brief Returns the classification adapter smart pointer of the model.
       *
       */
      const ClassificationAdapterSPtr classification() const;

      /** \brief Creates the root node of the categories.
       * \param[in] name name of the root category.
       *
       * Special Nodes of the model to refer different roots.
       *
       */
      CategoryAdapterSPtr createRootCategory(const QString& name);

       /** \brief Creates a category and returns it's category adapter smart pointer.
       * \param[in] name name of the category.
       * \param[in] parent raw pointer category adapter of the parent of the new category.
       *
       */
      CategoryAdapterSPtr createCategory(const QString& name, CategoryAdapterPtr  parent);

      /** \brief Creates a category and returns it's category adapter smart pointer.
       * \param[in] name name of the category.
       * \param[in] parent smart pointer category adapter of the parent of the new category.
      *
      */
      CategoryAdapterSPtr createCategory(const QString& name, CategoryAdapterSPtr parent);

      /** \brief Adds a category to the model.
       * \param[in] category smart pointer of the category adapter to add.
       * \param[in] parent smart pointer of the category adapter parent of the added category.
       *
       */
      void addCategory(CategoryAdapterSPtr category, CategoryAdapterSPtr parent);

      /** \brief Removes a category to the model.
       * \param[in] category smart pointer of the category adapter to remove.
       * \param[in] parent smart pointer of the category adapter parent of the added category.
       *
       */
      void removeCategory(CategoryAdapterSPtr category, CategoryAdapterSPtr parent);

      /** \brief Removes a category whose parent is the root node.
       * \param[in] category smart pointer of the category adapter to remove.
       *
       */
      void removeRootCategory(CategoryAdapterSPtr category);

      /** \brief Changes the parent of an existing category.
       * \param[in] category smart pointer of the category to change parent.
       * \param[in] parent smart pointer of the category that is the new parent.
       *
       * TODO 2013-10-21: Throw exception if they don't belong to the same classification
       *
       */
      void reparentCategory(CategoryAdapterSPtr category, CategoryAdapterSPtr parent);

      /** \brief Adds a sample to the model.
       * \param[in] sample smart pointer of the sample adapter to add.
       *
       */
      void add(SampleAdapterSPtr sample);

      /** \brief Adds a list of samples to the model.
       * \param[in] samples list of smart pointers of the sample adapters to add.
       *
       */
      void add(SampleAdapterSList samples);

      /** \brief Adds a channel to the model.
       * \param[in] channel smart pointer of the channel adapter to add.
       *
       */
      void add(ChannelAdapterSPtr channel);

      /** \brief Adds a list of channels to the model.
       * \param[in] channels list of smart pointers of the channels adapters to add.
       *
       */
      void add(ChannelAdapterSList channels);

      /** \brief Adds a segmentation to the model.
       * \param[in] segmentation smart pointer of the segmentation adapter to add.
       *
       */
      void add(SegmentationAdapterSPtr segmentation);

      /** \brief Adds a list of segmentations to the model.
       * \param[in] segmentations list of smart pointers of the segmentations adapters to add.
       *
       */
      void add(SegmentationAdapterSList segmentations);

      /** \brief Adds a relation between two item adapters in the model.
       * \param[in] ancestor item adapter smart pointer origin of the relation.
       * \param[in] successor item adapter smart pointer destination of the relation.
       * \param[in] relation text string that specifies the relation.
       *
       */
      void addRelation(ItemAdapterSPtr     ancestor,
                       ItemAdapterSPtr     successor,
                       const RelationName &relation);

      /** \brief Adds a relation to the model.
       * \param[in] relation valid relation object.
       *
       */
      void addRelation(const Relation& relation);

      /** \brief Adds a list of relations to the model.
       * \param[in] relationList list of Relation objects.
       *
       */
      void addRelations(const RelationList &relations);


      /** \brief Removes a sample from the model.
       * \param[in] sample smart pointer of the sample adapter to remove.
       *
       */
      void remove(SampleAdapterSPtr sample);

      /** \brief Removes a list of samples from the model.
       * \param[in] samples list of smart pointers of the sample adapters to remove.
       *
       */
      void remove(SampleAdapterSList samples);

      /** \brief Removes a channel from the model.
       * \param[in] channel smart pointer of the channel adapter to remove.
       *
       */
      void remove(ChannelAdapterSPtr channel);

      /** \brief Removes a list of channels from the model.
       * \param[in] channels list of smart pointers of the channel adapters to remove.
       *
       */
      void remove(const ChannelAdapterSList &channels);

      /** \brief Removes a segmentation from the model.
       * \param[in] segmentation smart pointer of the segmentation adapter to remove.
       *
       */
      void remove(const SegmentationAdapterSPtr segmentation);

      /** \brief Removes a list of segmentations from the model.
       * \param[in] segmentations list of smart pointers of the segmentation adapters to remove.
       *
       */
      void remove(const SegmentationAdapterSList &segmentations);

      void changeSpacing(ChannelAdapterSPtr channel, const NmVector3 &spacing);

      /** \brief Returns the list of sample adapters in the model.
       *
       */
      const SampleAdapterSList &samples() const
      { return m_samples; }

      /** \brief Returns the list of channel adapters in the model.
       *
       */
      const ChannelAdapterSList &channels() const
      { return m_channels; }

      /** \brief Returns the list of segmentation adapters in the model.
       *
       */
      const SegmentationAdapterSList &segmentations() const
      { return m_segmentations; }

      /** \brief Sets the category of a segmentation.
       * \param[in] segmentation smart pointer of the segmentation adapter to change.
       * \param[in] category smart pointer of the new category adapter.
       *
       */
      void setSegmentationCategory(SegmentationAdapterSPtr segmentation,
                                   const CategoryAdapterSPtr     category);



      /** \brief Removes a relation between two item adapters from the model.
       * \param[in] ancestor item adapter smart pointer origin of the relation.
       * \param[in] successor item adapter smart pointer destination of the relation.
       * \param[in] relation text string that specifies the relation.
       *
       */
      void deleteRelation(const ItemAdapterSPtr     ancestor,
                          const ItemAdapterSPtr     successor,
                          const RelationName &relation);

      /** \brief Deletes a relation from the model.
       * \param[in] relation valid relation object.
       *
       */
      void deleteRelation(const Relation& relation);

      /** \brief Deletes a list of relations to the model.
       * \param[in] relationList list of Relation objects.
       *
       */
      void deleteRelations(const RelationList &relations);

      /** \brief Returns the list of item adapters related to the specified one.
       * \param[in] item item adapter raw pointer.
       * \param[in] type type of the relation.
       * \param[in] filter relations filter.
       *
       */
      ItemAdapterSList relatedItems(ItemAdapterPtr item, RelationType type, const RelationName& filter = QString());

      /** \brief Returns the list of relations that an item have that comply with the specified type and filter.
       * \param[in] item item adapter raw pointer.
       * \param[in] type relation type.
       * \param[in] filter relations filter.
       *
       */
      RelationList relations(ItemAdapterPtr item, RelationType type, const RelationName& filter = QString());

      /** \brief Helper method that returns true if able to change the input of the root filter of the segmentation
       * given to the new given stack. Returns false on failure.
       * \param[in] segmentation Segmentation adapter.
       * \param[in] stack Channel adapter to change the segmentation.
       *
       */
      bool changeSegmentationStack(SegmentationAdapterPtr segmentation, ChannelAdapterPtr stack);

      //---------------------------------------------------------------------------
      /************************** Connections API ********************************/
      //---------------------------------------------------------------------------

      /** \brief Adds the given connection to the model.
       * \param[in] connection connection object.
       *
       */
      void addConnection(const Connection &connection);

      /** \brief Adds the given connections to the model.
       * \param[in] connection list of connections.
       *
       */
      void addConnections(const ConnectionList &connections);

      /** \brief Removes the given connection from the model.
       * \param[in] connection connection object.
       *
       */
      void deleteConnection(const Connection &connection);

      /** \brief Removes the given connections from the model.
       * \param[in] connections list of connections.
       *
       */
      void deleteConnections(const ConnectionList &connections);

      /** \brief Removes all the connections of the given segmentation from the model.
       * \param[in] segmentation segmentation object.
       *
       */
      void deleteConnections(const SegmentationAdapterSPtr segmentation);

      /** \brief Returns all the connection of the given segmentation.
       * \param[in] segmentation segmentation object.
       *
       */
      const ConnectionList connections(const SegmentationAdapterSPtr segmentation);

      /** \brief Returns all the connections between the given segmentations.
       * \param[in] segmentation1 segmentation object.
       * \param[in] segmentation2 segmentation object.
       *
       */
      const ConnectionList connections(const SegmentationAdapterSPtr segmentation1, SegmentationAdapterSPtr segmentation2);

      //---------------------------------------------------------------------------
      /************************** SmartPointer API *******************************/
      //---------------------------------------------------------------------------

      /** \brief Returns the item adapter smart pointer of the item specified by it persistent smart pointer.
       * \param[in] item persistent smart pointer.
       *
       */
      ItemAdapterSPtr find(PersistentSPtr item);

      /** \brief Returns the smart pointer of a category adapter given its raw pointer.
       * \param[in] category category adapter raw pointer.
       *
       */
      CategoryAdapterSPtr smartPointer(CategoryAdapterPtr category);

      /** \brief Returns the smart pointer of a sample adapter given its raw pointer.
       * \param[in] sample sample adapter raw pointer.
       *
       */
      virtual SampleAdapterSPtr smartPointer(SampleAdapterPtr sample);

      /** \brief Returns the smart pointer of a channel adapter given its raw pointer.
       * \param[in] channel channel adapter raw pointer.
       *
       */
      ChannelAdapterSPtr smartPointer(ChannelAdapterPtr channel);

      /** \brief Returns the smart pointer of a segmentation adapter given its raw pointer.
       * \param[in] segmentation segmentation adapter raw pointer.
       *
       */
      SegmentationAdapterSPtr smartPointer(SegmentationAdapterPtr segmentation);

      //---------------------------------------------------------------------------
      /************************** Locator API ************************************/
      //---------------------------------------------------------------------------

      /** \brief Returns the list of elements whose bounds contains the given point.
       * \param[in] point Point 3D coordinates.
       *
       */
      const ViewItemAdapterSList contains(const NmVector3 &point) const;

      /** \brief Returns the list of elements whose bounds intersects the given one.
       * \param[in] bounds Bounds object.
       *
       */
      const ViewItemAdapterSList intersects(const Bounds &bounds) const;

      /** \brief Rebuilds the DBVH tree.
       *
       */
      void rebuildLocator();

      /** \brief Returns a locator for the DBVH tree.
       *
       */
      SegmentationLocatorSPtr locator()
      { return std::move(std::make_shared<GUI::Model::Utils::SegmentationLocator>(&m_dbvh)); }

      //---------------------------------------------------------------------------
      /************************** Fixes API **************************************/
      //---------------------------------------------------------------------------

      /** \brief Helper method to move segmentations that are in the wrong channel to the
       *  designated primary channel. Needs to reassign the segmentations inputs and relations
       *  with the sample (Sample::CONTAINS). Normally this method shouldn't be called at all
       *  in production code and it's here just to solve momentary SEG file problems where users
       *  have segmented in several channels and find problems with the counting frame plugin later on.
       *
       */
      void fixChannels(ChannelAdapterPtr primary);

    signals:
      void classificationAdded  (ClassificationAdapterSPtr classification);
      void classificationRemoved(ClassificationAdapterSPtr classification);

      void samplesAdded  (SampleAdapterSList samples);
      void samplesRemoved(SampleAdapterSList samples);
      void samplesAboutToBeRemoved(SampleAdapterSList samples);

      void channelsAdded  (ViewItemAdapterSList channesl);
      void channelsRemoved(ViewItemAdapterSList  channels);
      void channelsAboutToBeRemoved(ViewItemAdapterSList channels);

      void segmentationsAdded  (ViewItemAdapterSList segmentations);
      void segmentationsRemoved(ViewItemAdapterSList segmentations);
      void segmentationsAboutToBeRemoved(ViewItemAdapterSList segmentations);

      void viewItemsAdded(ViewItemAdapterSList channesl);
      void viewItemsRemoved(ViewItemAdapterSList channesl);
      void viewItemsAboutToBeRemoved(ViewItemAdapterSList channesl);

      void connectionAdded(Connection connection);
      void connectionRemoved(Connection connection);

      void aboutToBeReset();

      void modelChanged();

    private slots:
      void resetInternalData();

    private:
      bool contains(ItemAdapterSPtr &item, const ItemCommandsList &list) const;

      int find(ItemAdapterSPtr &item, const ItemCommandsList &list) const;

      void remove(ItemAdapterSPtr &item, ItemCommandsList &list);

      void classifyQueues(const ItemCommandsList &queues,
                          ItemCommandsList       &samplesQueues,
                          ItemCommandsList       &channelQueues,
                          ItemCommandsList       &segmentationQueues);

      ViewItemAdapterSList queuedViewItems(const ItemCommandsList &queue) const;

      SampleAdapterSList queuedSamples(const ItemCommandsList &queue) const;

      void queueAddRelationCommand(ItemAdapterSPtr ancestor, ItemAdapterSPtr successor, const QString &relation);

      void queueAddConnectionCommand(const Connection &connection);

      void queueAddCommand(ItemAdapterSPtr item, BatchCommandSPtr command);

      void queueUpdateCommand(ItemAdapterSPtr item, BatchCommandSPtr command);

      void queueRemoveCommand(ItemAdapterSPtr item, BatchCommandSPtr command);

      void executeCommandsIfNoBatchMode();

      void executeAddCommands();

      void executeUpdateCommands();

      void executeRemoveCommands();

      void executeAddQueues(QModelIndex parent, ItemCommandsList &queueList);

      void executeUpdateQueues(ItemCommandsList &queueList);

      void executeRemoveQueues(QModelIndex parent, ItemCommandsList &queueList);

      /** \brief Group queue commands by consecutive indices
       *
       */
      ConsecutiveQueuesList groupConsecutiveQueues(ItemCommandsList &queueList);

      /** \brief Creates a command to add sample to the model
       * \param[in] sample to be added to the model
       *
       */
      BatchCommandSPtr addSampleCommand(SampleAdapterSPtr sample);

      /** \brief Creates a command to add channel to the model
       * \param[in] channel to be added to the model
       *
       */
      BatchCommandSPtr addChannelCommand(ChannelAdapterSPtr channel);

     /** \brief Creates a command to add segmentation to the model
       * \param[in] segmentation to be added to the model
       *
       */
      BatchCommandSPtr addSegmentationCommand(SegmentationAdapterSPtr segmentation);

     /** \brief Creates a command to add the relation to the model
       * \param[in] ancestor of the relation
       * \param[in] successor of the relation
       * \param[in] relation description name
       *
       */
      BatchCommandSPtr addRelationCommand(ItemAdapterSPtr     ancestor,
                                          ItemAdapterSPtr     successor,
                                          const RelationName &relation);

      /** \brief Removes a sample adapter from the model and its adapted sample from the analysis.
       * \param[in] sample sample adapter smart pointer.
       *
       */
      BatchCommandSPtr removeSampleCommand(SampleAdapterSPtr sample);

      /** \brief Removes a channel adapter from the model and its adapted channel from the analysis.
       * \param[in] channel channel adapter smart pointer.
       *
       */
      BatchCommandSPtr removeChannelCommand(ChannelAdapterSPtr channel);

      /** \brief Removes a segmentation adapter from the model and its adapted segmentation from the analysis.
       * \param[in] segmentation segmentation adapter smart pointer.
       *
       */
      BatchCommandSPtr removeSegmentationCommand(SegmentationAdapterSPtr segmentation);

      /** \brief Emits the connection removed signals.
       * \param[in] connections List of connections that will be removed later after signaling.
       *
       */
      BatchCommandSPtr removeConnectionsCommand(const ConnectionList connections);

    private:
      /** \class BatchCommand
       * \brief Modifies the model executing a group of commands in batch mode.
       *
       */
      class BatchCommand
      {
        public:
          /** \brief BatchCommand class constructor.
           *
           */
          explicit BatchCommand() {}

          /** \brief BatchCommand virtual destructor.
           *
           */
          virtual ~BatchCommand() {}

          /** \brief Executes the command.
           *
           */
          virtual void execute() = 0;
      };

      /** \class Command
       * \brief Contains an executable lambda.
       *
       */
      template <typename T>
      class Command
      : public BatchCommand
      {
        public:
          /** \brief Command class constructor.
           * \param[in] expression Expression element.
           *
           */
          explicit Command(T expression)
          : m_lambda(expression) {}

          /** \brief Executes the stored expression.
           *
           */
          virtual void execute() override
          { m_lambda();}

        private:
          T m_lambda; /** executable expression. */
      };

    private:
      AnalysisSPtr                m_analysis;        /** adapted analysis class.                        */
      SampleAdapterSList          m_samples;         /** list of sample adapters in the model.          */
      ChannelAdapterSList         m_channels;        /** list of channel adapters in the model.         */
      SegmentationAdapterSList    m_segmentations;   /** list of segmentation adapters in the model.    */
      ClassificationAdapterSPtr   m_classification;  /** adapter of classification of adapted analysis. */
      GUI::Model::Utils::DBVHNode m_dbvh;            /** spatial location tree.                         */

      QHash<const Persistent::Uuid, SegmentationAdapterSPtr> m_sptrLookup; /** relates pointers to smartpointers of segmentations for faster lookup based on common Uuid. */

      bool m_isBatchMode;                 /** true if the model is currently in batch mode operation. */
      ItemCommandsList m_addCommands;     /** pending add commands to process.                        */
      ItemCommandsList m_updateCommands;  /** pending update commands to process.                     */
      ItemCommandsList m_removeCommands;  /** pending remove commands to process.                     */
  };

  using ModelAdapterPtr  = ModelAdapter *;
  using ModelAdapterSPtr = std::shared_ptr<ModelAdapter>;

  /** \brief Returns true if the given index is an item adapter.
   * \param[in] index model index.
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
