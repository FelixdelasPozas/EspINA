/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2015  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef ESPINA_REPRESENTATION_POOL_H
#define ESPINA_REPRESENTATION_POOL_H

#include "GUI/EspinaGUI_Export.h"

// ESPINA
#include <Core/MultiTasking/Task.h>
#include <Core/Utils/Vector3.hxx>
#include <GUI/Representations/RangedValue.hxx>
#include <GUI/Model/ViewItemAdapter.h>
#include "RepresentationState.h"
#include "RepresentationPipeline.h"
#include "ModelSources.h"

// C++
#include <memory>
#include <atomic>

namespace ESPINA
{
  class EspinaGUI_EXPORT PoolSettings
  : public QObject
  {
    Q_OBJECT
  public:
    /** \brief PoolSettings class constructor.
     *
     */
    virtual ~PoolSettings()
    {}

    /** \brief Returns a RepresentationState object of this settings.
     *
     */
    RepresentationState settings();

    /** \brief Sets a value for a the given tag and signals the modification (if any).
     * \param[in] tag text string.
     * \param[in] value tag value.
     *
     */
    template<typename T>
    void set(const QString &tag, const T value)
    {
      if(m_state.getValue<T>(tag) != value)
      {
        m_state.setValue<T>(tag, value);

        if(m_state.hasPendingChanges())
        {
          emit modified();
        }
      }
    }

    /** \brief Returns the value for the given tag.
     * \param[in] tag text string.
     *
     */
    template<typename T>
    const T get(const QString &tag) const
    {
      return m_state.getValue<T>(tag);
    }

  signals:
    void modified();

  private:
    RepresentationState m_state; /** state maintained by the settings object. */
  };

  using PoolSettingsSPtr = std::shared_ptr<PoolSettings>;

  class EspinaGUI_EXPORT RepresentationPool
  : public QObject
  {
    Q_OBJECT
  public:
    /** \brief RepresentationPool class constructor.
     *
     */
    virtual ~RepresentationPool();

    /** \brief Sets the sources this pool will make representations for.
     * \param[in] sources items group.
     *
     */
    void setPipelineSources(PipelineSources *sources);

    /** \brief Returns the sources this pool attends to.
     *
     */
    ViewItemAdapterList sources() const;

    /** \brief Sets the settings for the pool's representations.
     * \param[in] settings pool settings.
     *
     */
    void setSettings(PoolSettingsSPtr settings);

    /** \brief Returns the pool's representations' settings.
     *
     */
    RepresentationState settings() const;

    /** \brief Modifies a settings tag with a new value.
     * \param[in] tag text string.
     * \param[in] value tag value.
     *
     */
    template<typename T>
    void setSetting(const QString &tag, const T value);

    /** \brief Update all representations to conform crosshair and resolution
     * \param[in] crosshair scene crosshair
     * \param[in] resolution scene resolution
     */
    void updatePipelines(const GUI::Representations::FrameCSPtr frame);

    /** \brief Returns the item picked by the specified point or is associated to the specified actor.
     * \param[in] point pick point coordinates.
     * \param[in] actor suggested actor belonging to picked item.
     *
     */
    virtual ViewItemAdapterList pick(const NmVector3 &point, vtkProp *actor) const = 0;

    /** \brief Returns the range of frames for which there are representations.
     *
     */
    TimeRange readyRange() const;

    /** \brief Returns the timestamp of the latest frame for which representations are ready.
     *
     */
    TimeStamp lastUpdateTimeStamp() const;

    /** \brief Returns if the pool has sources to generate actors from
     *
     */
    bool hasSources() const;

    /** \brief Returns all valid actors for the given time.
     * \param[in] t timestamp.
     *
     */
    RepresentationPipeline::Actors actors(TimeStamp t) const;

    /** \brief Invalidates all actors previous a given time.
     * \param[in] t timestamp.
     *
     */
    void invalidatePreviousActors(TimeStamp t);

    /** \brief Reuses the last ready representations for the given frame.
     * \param[in] frame const frame object.
     */
    void reuseRepresentations(const GUI::Representations::FrameCSPtr frame);

    /** \brief Increment the number of active managers using this pool
     *
     */
    void incrementObservers();

    /** \brief Decrement the number of active managers using this pool
     *
     */
    void decrementObservers();

    /** \brief Invalidates the specified representations.
     * \param[in] items representations to invalidate.
     * \param[in] t timestamp of the new actors after invalidation.
     *
     */
    void invalidateRepresentations(ViewItemAdapterList items, GUI::Representations::FrameCSPtr frame);

    /** \brief Returns the type of items of the pool.
     *
     */
    ItemAdapter::Type type() const;

  signals:
    /** \brief Some managers may be interested in changes in the actors of the pool
     *
     *   This signal is only emitted whenever two consecutive time stamps generate
     *   different actors
     */
    void actorsReady(const GUI::Representations::FrameCSPtr frame);

    void actorsInvalidated(const GUI::Representations::FrameCSPtr frame);

    void taskStarted(TaskSPtr task);

  protected:
    /** \brief RepresentationPool class constructor.
     * \param[in] type type of the sources this pool will attend to.
     *
     */
    explicit RepresentationPool(const ItemAdapter::Type &type);

    /** \brief Returns true if there hasn't been generated the actors for the given time.
     * \param[in] t timestamp.
     *
     */
    bool notHasBeenProcessed(const TimeStamp t) const;

    /** \brief Returns true if the pool is being managed and has sources
     *
     */
    bool isEnabled() const;

  protected slots:
    /** \brief Stores the actors timestamp and signals the event to observers.
     * \param[in] frame frame object.
     * \param[in] actors list of actors for the given frame of the attended sources.
     *
     */
    void onActorsReady(const GUI::Representations::FrameCSPtr frame, RepresentationPipeline::Actors actors);

  private slots:
    /** \brief Adds the pipelines to the corresponding added source items, and updates the representations.
     * \param[in] sources added items.
     * \param[in] frame frame of the moment of addition.
     *
     */
    void onSourcesAdded(ViewItemAdapterList sources, const GUI::Representations::FrameCSPtr frame);

    /** \brief Removes the pipelines of the removed source items, and updates the representations.
     * \param[in] sources removed items.
     * \param[in] frame frame of the moment of removal.
     *
     */
    void onSourcesRemoved(ViewItemAdapterList sources, const GUI::Representations::FrameCSPtr frame);

    /** \brief Recomputes the actors when the sources have changed.
     * \param[in] sources modified items.
     * \param[in] frame frame object of the moment of modification.
     *
     */
    void onSourcesInvalidated(ViewItemAdapterList sources, const GUI::Representations::FrameCSPtr frame);

    /** \brief Recomputes the actors when the sources properties have changed.
     * \param[in] sources modified items.
     * \param[in] frame frame object of the moment of modification.
     *
     */
    void onSourceColorsInvalidated(ViewItemAdapterList sources, const GUI::Representations::FrameCSPtr frame);

    /** \brief Updates the pipelines when the settings of the pool changes.
     *
     */
    void onSettingsModified();

  private:
    /** \brief Adds a pipeline for the given item in all the updaters of the pool.
     * \param[in] source item.
     *
     */
    virtual void addRepresentationPipeline(ViewItemAdapterPtr source) = 0;

    /** \brief Removes the pipeline for the given item from all the updaters of the pool.
     * \param[in] source item.
     *
     */
    virtual void removeRepresentationPipeline(ViewItemAdapterPtr source) = 0;

    /** \brief Updates the settings of all the updaters of the pool.
     *
     */
    virtual void updatePipelinesImplementation(const GUI::Representations::FrameCSPtr frame) = 0;

    /** \brief Applies the settings to all the updaters of the pool.
     *
     */
    virtual void applySettings(const RepresentationState &settings) = 0;

    /** \brief Creates the representations for the given modified items in the given frame.
     * \param[in] frame const frame object.
     * \param[in] modifiedItems list of modified items.
     *
     */
    void updateRepresentationsAt(const GUI::Representations::FrameCSPtr frame, ViewItemAdapterList modifiedItems = ViewItemAdapterList());

    /** \brief Updates the representations for the given modified items in the given frame.
     * \param[in] frame const frame object.
     * \param[in] modifiedItems list of modified items.
     *
     */
    void updateRepresentationColorsAt(const GUI::Representations::FrameCSPtr frame, ViewItemAdapterList modifiedItems = ViewItemAdapterList());

    /** \brief Implementation of updateRepresentationsAt of the subclasses.
     * \param[in] frame const frame object.
     * \param[in] modifiedItems list of modified items.
     *
     */
    virtual void updateRepresentationsAtImlementation(const GUI::Representations::FrameCSPtr frame, ViewItemAdapterList modifiedItems) = 0;

    /** \brief Implementation of updateRepresentationColorsAt of the subclasses.
     * \param[in] frame const frame object.
     * \param[in] modifiedItems list of modified items.
     *
     */
    virtual void updateRepresentationColorsAtImlementation(const GUI::Representations::FrameCSPtr frame, ViewItemAdapterList modifiedItems) = 0;

    /** \brief Returns true if the given actors are different from the ones in the last frame.
     * \param[in] actors list of actors.
     *
     */
    bool actorsChanged(RepresentationPipeline::Actors actors) const;

    /** \brief Returns true if there are unprocessed sources.
     *
     */
    bool hasPendingSources() const;

    /** \brief Adds the given items to the list pending to process.
     * \param[in] sources list of items.
     *
     */
    void addSources(ViewItemAdapterList sources);

    /** \brief Removes the given items from the list of pending to process or, if already processed removes the pipelines.
     * \param[in] sources list of items.
     *
     */
    bool removeSources(ViewItemAdapterList sources);

    /** \brief Adds the pipelines to the updaters from the items pending to process.
     *
     */
    void processPendingSources();

  private:
    ItemAdapter::Type m_type;                                  /** type of items to attend to. */
    PipelineSources  *m_sources;                               /** list of items. */

    PoolSettingsSPtr    m_settings;                            /** pool settings. */
    RepresentationState m_poolState;                           /** current pool state. */

    ViewItemAdapterList m_pendingSources;                      /** list of items pending to add a pipeline to the updaters. */

    RangedValue<RepresentationPipeline::Actors> m_validActors; /** range of frames and corresponding actors. */

    std::atomic<unsigned> m_numObservers;                      /** number of managers using the pool. */
    std::atomic<unsigned> m_sourcesCount;                      /** number of items observing. */
  };

  template<typename T>
  void RepresentationPool::setSetting(const QString &tag, const T value)
  {
    m_poolState.setValue<T>(tag, value);

    if (m_poolState.isModified(tag))
    {
      applySettings(m_poolState);
      m_poolState.commit();
    }
  }


  using RepresentationPoolSPtr  = std::shared_ptr<RepresentationPool>;
  using RepresentationPoolSList = QList<RepresentationPoolSPtr>;
} // namespace ESPINA

#endif // ESPINA_REPRESENTATION_POOL_H
