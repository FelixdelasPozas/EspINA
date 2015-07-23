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

#ifndef ESPINA_REPRESENTATION_UPDATER_H
#define ESPINA_REPRESENTATION_UPDATER_H

#include <Core/MultiTasking/Task.h>

#include <GUI/Model/ViewItemAdapter.h>
#include "RepresentationState.h"

namespace ESPINA
{

  class RepresentationUpdater
  : public Task
  {
    Q_OBJECT
  public:
    /** \brief Task to creates actors for sources using pipeline bounded to scheduler
     *
     */
    explicit RepresentationUpdater(SchedulerSPtr scheduler, RepresentationPipelineSPtr pipeline);

    /** \brief Start creating actors for item on execution
     *
     */
    void addSource(ViewItemAdapterPtr item);

    /** \brief Stop creating actors for item on execution
     *
     */
    void removeSource(ViewItemAdapterPtr item);

    /** \brief Returns if the representation updater has sources
     *
     */
    bool isEmpty() const;

    /** \brief Changes the crosshair point to be used for this representation
     *
     */
    void setCrosshair(const NmVector3 &point);

    /** \brief Changes the resolution to be used for this representation
     *
     */
    void setResolution(const NmVector3 &resolution);

    /** \brief Set the external settings to be used on actor creation
     *
     */
    void setSettings(const RepresentationState &settings);

    /** \brief Limits the number of sources to create actors on an execution
     *
     */
    void setUpdateList(ViewItemAdapterList sources);

    /** \brief Sets the execution TimeStamp of the task
     *
     */
    void setTimeStamp(TimeStamp time);

    /** \brief Returns the execution TimeStamp of the task
     *
     */
    TimeStamp timeStamp() const;

    /** \brief Invalidates the execution TimeStamp of the task
     *
     */
    void invalidate();

    /** \brief Returns if task has a valid TimeStamp
     *
     */
    bool hasValidTimeStamp() const;

    ViewItemAdapterList pick(const NmVector3 &point, vtkProp *actor) const;

    /** \brief Returns the latest actors computed by the task
     *
     */
    RepresentationPipeline::Actors actors() const;

  signals:
    void actorsReady(TimeStamp time, RepresentationPipeline::Actors actors);

  protected:
    virtual void run();

  private:
    RepresentationPipelineSPtr sourcePipeline(ViewItemAdapterPtr item) const;

    ViewItemAdapterPtr findActorItem(vtkProp *actor) const;

  private:
    TimeStamp m_timeStamp;
    bool      m_timeStampValid;

    RepresentationPipelineSPtr m_pipeline;

    mutable QMutex m_mutex;
    ViewItemAdapterList m_requestedSources;
    ViewItemAdapterList m_sources;
    ViewItemAdapterList *m_updateList;

    RepresentationState            m_settings;
    RepresentationPipeline::Actors m_actors;
  };

  using RepresentationUpdaterSPtr  = std::shared_ptr<RepresentationUpdater>;
  using RepresentationUpdaterSList = QList<RepresentationUpdaterSPtr>;
}

#endif // ESPINA_REPRESENTATIONUPDATER_H
