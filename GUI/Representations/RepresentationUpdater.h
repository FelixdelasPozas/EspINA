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
    explicit RepresentationUpdater(SchedulerSPtr scheduler, RepresentationPipelineSPtr pipeline);

    void addSource(ViewItemAdapterPtr item);

    void removeSource(ViewItemAdapterPtr item);

    void setCrosshair(const NmVector3 &point);

    bool applySettings(const RepresentationState &setting);

    void setTimeStamp(TimeStamp time);

    TimeStamp timeStamp() const;

    void invalidate();

    bool hasValidTimeStamp() const;

    RepresentationPipeline::ActorList actors() const;

  signals:
    void actorsUpdated(TimeStamp time, RepresentationPipeline::ActorList pipelines);

  protected:
    virtual void run();

  private:
    RepresentationPipelineSPtr sourcePipeline(ViewItemAdapterPtr item) const;

  private:
    TimeStamp m_timeStamp;
    bool      m_timeStampValid;

    RepresentationPipelineSPtr m_pipeline;
    QMap<ViewItemAdapterPtr, RepresentationState> m_states;

    RepresentationPipeline::ActorList m_actors;
  };

  using RepresentationUpdaterSPtr  = std::shared_ptr<RepresentationUpdater>;
  using RepresentationUpdaterSList = QList<RepresentationUpdaterSPtr>;
}

#endif // ESPINA_REPRESENTATIONUPDATER_H
