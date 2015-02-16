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

#ifndef ESPINA_INFORMATION_PROXY_H
#define ESPINA_INFORMATION_PROXY_H

#include "GUI/EspinaGUI_Export.h"

// ESPINA
#include <Core/MultiTasking/Task.h>
#include <GUI/Model/ModelAdapter.h>

// Qt
#include <QAbstractProxyModel>
#include <QStringList>

namespace ESPINA
{

  class EspinaGUI_EXPORT InformationProxy
  : public QAbstractProxyModel
  {
    Q_OBJECT

  protected:
    /** \brief Helper method to return the name tag.
     *
     */
    static SegmentationExtension::InfoTag NameTag()
    { return QObject::tr("Name"); }

    /** \brief Helper method to return the category tag.
     *
     */
    static SegmentationExtension::InfoTag CategoryTag()
    { return QObject::tr("Category"); }

    class InformationFetcher
    : public Task
    {
    public:
      /** \brief InformationFetcher class constructor.
       * \param[in] segmentation, adapter smart pointer of the segmentation to get information from.
       * \param[in] tags, information tags to fetch.
       * \param[in] scheduler, scheduler smart pointer.
       *
       */
      InformationFetcher(SegmentationAdapterPtr segmentation,
                         const SegmentationExtension::InfoTagList &tags,
                         SchedulerSPtr scheduler)
      : Task        {scheduler}
      , Segmentation{segmentation}
      , m_tags      {tags}
      , m_progress  {0}
      {
        auto id = Segmentation->data(Qt::DisplayRole).toString();
        setDescription(tr("%1 information").arg(id));
        setHidden(true);

        m_tags.removeOne(NameTag());
        m_tags.removeOne(CategoryTag());

        bool ready = true;

        for (auto tag : m_tags)
        {
          ready &= Segmentation->isInformationReady(tag);

          if (!ready) break;
        }

        setFinished(ready);
      }

    public:
      /** \brief Returns current progress.
       *
       */
      int currentProgress() const
      { return m_progress; }

    protected:
      /** \brief Implements Task::run().
       *
       */
      virtual void run()
      {
        for (int i = 0; i < m_tags.size(); ++i)
        {
          if (!canExecute()) break;

          auto tag = m_tags[i];
          if (tag != NameTag() && tag != CategoryTag())
          {
            if (!Segmentation->isInformationReady(tag))
            {
              Segmentation->information(tag);

              if (!canExecute()) break;
            }
          }

          m_progress = (100.0*i)/m_tags.size();
          emit progress(m_progress);
        }
      }

    protected:
      SegmentationAdapterPtr Segmentation;
      SegmentationExtension::InfoTagList m_tags;
      int   m_progress;
    };

    using InformationFetcherSPtr = std::shared_ptr<InformationFetcher>;

  public:
    /** \brief InformationProxy class constructor.
     * \param[in] scheduler, scheduler smart pointer.
     *
     */
    explicit InformationProxy(SchedulerSPtr scheduler);

    /** \brief InformationProxy class virtual destructor.
     *
     */
    virtual ~InformationProxy();

    /** \brief Sets the model of the proxy.
     * \param[in] sourceModel, model adapter smart pointer.
     *
     */
    virtual void setSourceModel(ModelAdapterSPtr sourceModel);

    /** \brief Returns the item index of the proxy from the item index of the source.
     * \param[in] sourceIndeox, QModelIndex object.
     *
     */
    virtual QModelIndex mapFromSource(const QModelIndex& sourceIndex) const;

    /** \brief Returns the item index of the model from the item index of the proxy.
     * \param[in] proxyIndex, QModelIndex object.
     *
     */
    virtual QModelIndex mapToSource(const QModelIndex& proxyIndex) const;

    /** \brief Returns the number of columns.
     * \param[in] parent, QModelIndex object.
     *
     */
    virtual int columnCount(const QModelIndex& parent = QModelIndex()) const;

    /** \brief Returns the number of rows.
     * \param[in] parent, QModelIndex object.
     *
     */
    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;

    /** \brief Returns the item index of the parent of the given index.
     * \param[in] chind, QModelIndex object.
     *
     */
    virtual QModelIndex parent(const QModelIndex& child) const;

    /** \brief Returns the index of an element given the row, column and parent.
     * \param[in] row
     * \param[in] column
     * \param[in] parent, QModelIndex object.
     *
     */
    virtual QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const;

    /** \brief Overrides QAbstractProxyModel::headerData().
     *
     */
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    /** \brief Overrides QAbstractProxyModel::data()
     *
     */
    virtual QVariant data(const QModelIndex& proxyIndex, int role = Qt::DisplayRole) const override;

    /** \brief Sets the category.
     * \param[in] classificationName, name of the category.
     *
     */
    void setCategory(const QString &classificationName);

    /** \brief Returns the category.
     *
     */
    QString category() const
    { return m_category; }

    /** \brief Sets the segmaentations to show.
     * \param[in] filter, list of segmentation adapter raw pointers.
     *
     */
    void setFilter(const SegmentationAdapterList *filter);

    /** \brief Sets the information tags for the columns.
     * \param[in] tags, segmentation extension information tags.
     *
     */
    virtual void setInformationTags(const SegmentationExtension::InfoTagList tags);

    /** \brief Returns the information tags.
     *
     */
    const QStringList informationTags() const
    { return m_tags; }

    /** \brief Returns the list of item adapters currently displayed.
     *
     */
    ItemAdapterList displayedItems() const
    { return m_elements; }

    /** \brief Returns general progress count.
     *
     */
    int progress() const;

  signals:
    void informationProgress();

  protected slots:
    /** \brief Perform operations before and after the insertion of rows in the model.
     * \param[in] sourceParent index of the parent to add elements.
     * \param[in] start start row.
     * \param[in] end end row.
     *
     */
    void sourceRowsInserted(const QModelIndex & sourceParent, int start, int end);

    /** \brief Perform operations before and after the deletion of rows in the model.
     * \param[in] sourceParent index of the parent to add elements.
     * \param[in] start start row.
     * \param[in] end end row.
     *
     */
    void sourceRowsAboutToBeRemoved(const QModelIndex & sourceParent, int start, int end);

    /** \brief Perform operations before and after the modification of data in the model.
     * \param[in] sourceTopLeft, top left index.
     * \param[in] sourceBottomRight, bottom right index.
     *
     */
    void sourceDataChanged(const QModelIndex& sourceTopLeft, const QModelIndex& sourceBottomRight);

    /** \brief Removes all the elements in the model.
     *
     */
    void sourceModelReset();

    /** \brief Reports progress.
     * TODO
     *
     */
    void onProgessReported(int progress);

    /** \brief Reports progress.
     * TODO
     *
     */
    void onTaskFininished();

  private:
    /** \brief Returns true if the segmentation should be in the proxy model.
     *
     */
    bool acceptSegmentation(const SegmentationAdapterPtr segmentation) const;

    /** \brief Returns the proxy model index of a given item adapter.
     * \param[in] segmentation, item adapter raw pointer.
     * \param[in] col, column.
     *
     */
    QModelIndex index(const ItemAdapterPtr segmentation, int col = 0);

  protected:
    SegmentationExtension::InfoTagList m_tags;
    mutable QMap<SegmentationAdapterPtr, InformationFetcherSPtr> m_pendingInformation;
    SchedulerSPtr m_scheduler;

  private:
    ModelAdapterSPtr m_model;
    QString          m_category;

    const SegmentationAdapterList     *m_filter;

    ItemAdapterList m_elements;
  };
} // namespace ESPINA

#endif // INFORMATIONPROXY_H
