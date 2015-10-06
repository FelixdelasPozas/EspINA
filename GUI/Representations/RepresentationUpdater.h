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
#include <GUI/Types.h>

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

    /** \brief Add sources representations to be update
     *
     */
    void updateRepresentations(ViewItemAdapterList sources);

    void updateRepresentationColors(const ViewItemAdapterList &sources);

    /** \brief Sets the execution TimeStamp of the task
     *
     */
    void setFrame(const GUI::Representations::FrameCSPtr frame);

    /** \brief Returns the execution TimeStamp of the task
     *
     */
    GUI::Representations::FrameCSPtr frame() const;

    /** \brief Invalidates the execution TimeStamp of the task
     *
     */
    void invalidate();

    ViewItemAdapterList pick(const NmVector3 &point, vtkProp *actor) const;

    /** \brief Returns the latest actors computed by the task
     *
     */
    RepresentationPipeline::Actors actors() const;

  signals:
    void actorsReady(const GUI::Representations::FrameCSPtr frame, RepresentationPipeline::Actors actors);

  protected:
    virtual void run();

  private:
    using UpdateRequest     = QPair<ViewItemAdapterPtr, bool>;
    using UpdateRequestList = QList<UpdateRequest>;

    RepresentationPipelineSPtr sourcePipeline(ViewItemAdapterPtr item) const;

    ViewItemAdapterPtr findActorItem(vtkProp *actor) const;

    void updateRepresentations(ViewItemAdapterList sources, const bool createActors);

    static void addUpdateRequest(UpdateRequestList &list, ViewItemAdapterPtr item, const bool createActors);

    static void removeUpdateRequest(UpdateRequestList &list, ViewItemAdapterPtr item);

  private:
    GUI::Representations::FrameCSPtr m_frame;

    RepresentationPipelineSPtr m_pipeline;

    mutable QMutex m_mutex;

    UpdateRequestList  m_requestedSources;
    UpdateRequestList  m_sources;
    UpdateRequestList *m_updateList;

    RepresentationState            m_settings;
    RepresentationPipeline::Actors m_actors;
  };

  using RepresentationUpdaterSPtr  = std::shared_ptr<RepresentationUpdater>;
  using RepresentationUpdaterSList = QList<RepresentationUpdaterSPtr>;
}

#endif // ESPINA_REPRESENTATIONUPDATER_H
