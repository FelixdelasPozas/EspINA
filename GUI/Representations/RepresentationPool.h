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
#include "RepresentationPipeline.h"
#include "PipelineSources.h"

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

      virtual RepresentationPipeline::State pipelineSettings() = 0;
    };

    using SettingsSPtr = std::shared_ptr<Settings>;

  public:
    virtual ~RepresentationPool();

    void setPipelineSources(PipelineSources *sources);

    void setSettings(SettingsSPtr settings);

    RepresentationPipeline::State settings() const;

    /** \brief Updates pool representation pipelines to the given position
     *
     */
    void setCrosshair(const NmVector3 &point, TimeStamp t);

    /** \brief Sets the resolution to be used for its representations
     *
     */
    virtual void setResolution(const NmVector3 &resolution) = 0;

    void update();

    /** \brief Returns whether all pipeline representations are set to the
     *         current position or not
     *
     */
    TimeRange readyRange() const;

    /** \brief Returns all valid actors for the given time
     *
     */
    RepresentationPipeline::ActorList actors(TimeStamp time);

    /** \brief Increment the number of active managers using this pool
     *
     */
    void incrementObservers();

    /** \brief Decrement the number of active managers using this pool
     *
     */
    void decrementObservers();

  signals:
    void actorsReady(TimeStamp time);

  protected:
    explicit RepresentationPool();

    /** \brief Returns whether the pool representations are displayed by
     *         at least one representation manager
     */
    bool isBeingUsed() const;

    ViewItemAdapterList sources() const;

    void invalidateActors(TimeStamp time);

    void invalidateActors();

  protected slots:
    void onActorsReady(TimeStamp time, RepresentationPipeline::ActorList actors);
    
  private slots:
    void onSourceAdded (ViewItemAdapterPtr source);
    void onSourcesAdded(ViewItemAdapterList sources);

    void onSourceRemoved (ViewItemAdapterPtr source);
    void onSourcesRemoved(ViewItemAdapterList sources);

    void onSourceUpdated (ViewItemAdapterPtr source);
    void onSourcesUpdated(ViewItemAdapterList sources);

  private:
    virtual void addRepresentationPipeline(ViewItemAdapterPtr source) = 0;

    virtual void setCrosshairImplementation(const NmVector3 &point, TimeStamp time) = 0;

    virtual void updateImplementation() = 0;

    bool hasPendingSources() const;

    void processPendingSources();

  private:
    PipelineSources *m_sources;
    SettingsSPtr     m_settings;

    ViewItemAdapterList m_pendingSources;

    NmVector3 m_crosshair;
    TimeStamp m_requestedTimeStamp;
    TimeStamp m_lastUpdateTimeStamp;

    QMap<TimeStamp, RepresentationPipeline::ActorList> m_actors;

    unsigned m_numObservers;
  };

  using RepresentationPoolSPtr  = std::shared_ptr<RepresentationPool>;
  using RepresentationPoolSList = QList<RepresentationPoolSPtr>;
} // namespace ESPINA

#endif // ESPINA_REPRESENTATION_POOL_H
