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
#include "RepresentationState.h"
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

      RepresentationState poolSettings();

      template<typename T>
      void set(const QString &tag, const T value)
      {
        m_state.setValue<T>(tag, value);
      }

    private:
      virtual RepresentationState poolSettingsImplementation();

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

    /** \brief Updates pool representation actors to the given position
     *
     */
    void setCrosshair(const NmVector3 &point, TimeStamp t);

    /** \brief Sets the resolution to be used for its representations
     *
     */
    virtual void setResolution(const NmVector3 &resolution) = 0;

    virtual ViewItemAdapterPtr pick(const NmVector3 &point, vtkProp *actor) const = 0;

    /** \brief Returns whether all pipeline representations are set to the
     *         current position or not
     *
     */
    TimeRange readyRange() const;

    /** \brief Returns all valid actors for the given time
     *
     */
    RepresentationPipeline::Actors actors(TimeStamp time);

    void invalidatePreviousActors(TimeStamp time);

    TimeStamp lastUpdateTimeStamp() const;

    /** \brief Increment the number of active managers using this pool
     *
     */
    void incrementObservers();

    /** \brief Decrement the number of active managers using this pool
     *
     */
    void decrementObservers();

  public slots:
    void invalidate();

  signals:
    /** \brief Some managers may be interested in changes in the actors of the pool
     *
     *   This signal is only emitted whenever two consecutive time stamps generate
     *   different actors
     */
    void actorsReady(TimeStamp time);

    /** \brief Some managers may be interested in pool updates
     *
     */
    void poolUpdated(TimeStamp time);

    void actorsInvalidated();

  protected:
    explicit RepresentationPool();

    /** \brief Returns whether the pool representations are displayed by
     *         at least one representation manager
     */
    bool isBeingUsed() const;

    ViewItemAdapterList sources() const;

  protected slots:
    void onActorsReady(TimeStamp time, RepresentationPipeline::Actors actors);

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

    virtual void onSettingsChanged(const RepresentationState &settings) = 0;

    virtual bool changed() const = 0;

    virtual void invalidateImplementation() = 0;

    void invalidateActors();

    bool hasPendingSources() const;

    void processPendingSources();

    void update();

  private:
    PipelineSources *m_sources;
    SettingsSPtr     m_settings;

    ViewItemAdapterList m_pendingSources;

    NmVector3 m_crosshair;
    TimeStamp m_requestedTimeStamp;
    TimeStamp m_lastUpdateTimeStamp;

    TimeRange m_validActorsTimes;
    QMap<TimeStamp, RepresentationPipeline::Actors> m_actors;

    unsigned m_numObservers;
  };

  template<typename T>
  void RepresentationPool::setSetting ( const QString &tag, const T value )
  {
    m_settings->set<T>(tag, value);

    if (isBeingUsed())
    {
      onSettingsChanged(m_settings->poolSettings());
    }
  }


  using RepresentationPoolSPtr  = std::shared_ptr<RepresentationPool>;
  using RepresentationPoolSList = QList<RepresentationPoolSPtr>;
} // namespace ESPINA

#endif // ESPINA_REPRESENTATION_POOL_H
