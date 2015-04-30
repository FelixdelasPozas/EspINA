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

#include <Core/Utils/NmVector3.h>
#include <GUI/Representations/RangedValue.hxx>
#include "RepresentationState.h"
#include "RepresentationPipeline.h"
#include "PipelineSourcesFilter.h"
#include <GUI/Model/ViewItemAdapter.h>
#include <memory>

namespace ESPINA
{
  class RepresentationPool
  : public QObject
  {
    Q_OBJECT

  public:
    class Settings
    {
    public:
      virtual ~Settings() {}

      RepresentationState poolSettings();

      template<typename T>
      void set(const QString &tag, const T value)
      {
        m_state.setValue<T>(tag, value);
      }

    private:
      virtual RepresentationState poolSettingsImplementation() const;

    private:
      RepresentationState m_state;
    };

    using SettingsSPtr = std::shared_ptr<Settings>;

  public:
    virtual ~RepresentationPool();

    void setPipelineSources(PipelineSources *sources);

    void setSettings(SettingsSPtr settings);

    RepresentationState settings() const;

    template<typename T>
    void setSetting(const QString &tag, const T value);

    /** \brief Update all representations to conform crosshair and resolution
     * \param[in] crosshair scene crosshair
     * \param[in] resolution scene resolution
     */
    void updatePipelines(const NmVector3 &crosshair, const NmVector3 &resolution, TimeStamp t);

    /** \brief Updates pool representation actors to the given position
     *
     */
    void setCrosshair(const NmVector3 &crosshair, TimeStamp t);

    /** \brief Sets the resolution to be used for its representations
     *
     */
    void setSceneResolution(const NmVector3 &resolution, TimeStamp t);

    virtual ViewItemAdapterPtr pick(const NmVector3 &point, vtkProp *actor) const = 0;

    /** \brief Returns whether all pipeline representations are set to the
     *         current position or not
     *
     */
    TimeRange readyRange() const;

    TimeStamp lastUpdateTimeStamp() const;

    /** \brief Returns if the pool has sources to generate pipelines from
     *
     */
    bool hasSources() const;

    /** \brief Returns all valid actors for the given time
     *
     */
    RepresentationPipeline::Actors actors(TimeStamp t);

    void invalidatePreviousActors(TimeStamp t);

    void reuseRepresentations(TimeStamp t);

    /** \brief Increment the number of active managers using this pool
     *
     */
    void incrementObservers();

    /** \brief Decrement the number of active managers using this pool
     *
     */
    void decrementObservers();

  signals:
    /** \brief Some managers may be interested in changes in the actors of the pool
     *
     *   This signal is only emitted whenever two consecutive time stamps generate
     *   different actors
     */
    void actorsReady(TimeStamp t);

    void actorsInvalidated();

  protected:
    explicit RepresentationPool();

    ViewItemAdapterList sources() const;

    bool notHasBeenProcessed(const TimeStamp t) const;

  protected slots:
    void onActorsReady(TimeStamp time, RepresentationPipeline::Actors actors);

  private slots:
    void onSourcesAdded(ViewItemAdapterList sources, TimeStamp t);

    void onSourcesRemoved(ViewItemAdapterList sources, TimeStamp t);

    void onRepresentationModified(ViewItemAdapterList sources, TimeStamp t);

    void onTimeStampUpdated(TimeStamp t);

    void onRepresentationsInvalidated(ViewItemAdapterPtr item);

  private:
    virtual void addRepresentationPipeline(ViewItemAdapterPtr source) = 0;

    virtual void removeRepresentationPipeline(ViewItemAdapterPtr source) = 0;

    virtual void setCrosshairImplementation(const NmVector3 &crosshair, TimeStamp t) = 0;

    virtual void setSceneResolutionImplementation(const NmVector3 &resolution, TimeStamp t) = 0;

    virtual void updatePipelinesImplementation(const NmVector3 &crosshair, const NmVector3 &resolution, TimeStamp t) = 0;

    virtual void onSettingsChanged(const RepresentationState &settings) = 0;

    void updateRepresentationsAt(TimeStamp t, ViewItemAdapterList modifiedItems = ViewItemAdapterList());

    virtual void updateRepresentationsImlementationAt(TimeStamp t, ViewItemAdapterList modifiedItems) = 0;

    bool actorsChanged(const RepresentationPipeline::Actors &actors) const;

    bool hasPendingSources() const;

    void addSources(ViewItemAdapterList sources);

    bool removeSources(ViewItemAdapterList sources);

    void processPendingSources();

    /** \brief Returns true if the pool is being managed and has sources
     *
     */
    bool isEnabled() const;

  private:
    PipelineSources    *m_sources;

    SettingsSPtr        m_settings;
    RepresentationState m_poolState;

    NmVector3 m_crosshair;
    NmVector3 m_resolution;

    ViewItemAdapterList m_pendingSources;

    RangedValue<RepresentationPipeline::Actors> m_validActors;

    unsigned m_numObservers;
    unsigned m_sourcesCount;
  };

  template<typename T>
  void RepresentationPool::setSetting(const QString &tag, const T value)
  {
    m_poolState.setValue<T>(tag, value);

    if (m_poolState.isModified(tag))
    {
      onSettingsChanged(m_poolState);
      m_poolState.commit();
    }
  }


  using RepresentationPoolSPtr  = std::shared_ptr<RepresentationPool>;
  using RepresentationPoolSList = QList<RepresentationPoolSPtr>;
} // namespace ESPINA

#endif // ESPINA_REPRESENTATION_POOL_H
