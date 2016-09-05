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

#include <GUI/EspinaGUI_Export.h>

// ESPINA
#include "RepresentationState.h"
#include <Core/MultiTasking/Task.h>
#include <GUI/Model/ViewItemAdapter.h>
#include <GUI/Types.h>

namespace ESPINA
{
  /** \class RepresentationUpdater
   * \brief Task to create or modificate representations.
   *
   */
  class EspinaGUI_EXPORT RepresentationUpdater
  : public Task
  {
      Q_OBJECT
    public:
      /** \brief RepresentationUpdate class constructor.
       * \param[in] scheduler task scheduler.
       * \param[in] pipeline generator of the actors of the items.
       *
       */
      explicit RepresentationUpdater(SchedulerSPtr scheduler, RepresentationPipelineSPtr pipeline);

      /** \brief Adds an item to create actors for.
       * \param[in] item view item.
       *
       */
      void addSource(ViewItemAdapterPtr item);

      /** \brief Removes an item and stops creating actors for it.
       * \param[in] item view item.
       *
       */
      void removeSource(ViewItemAdapterPtr item);

      /** \brief Returns true if the representation updater has sources and false otherwise.
       *
       */
      bool isEmpty() const;

      /** \brief Changes the crosshair point to be used for this representation.
       * \param[in] point crosshair point.
       *
       */
      void setCrosshair(const NmVector3 &point);

      /** \brief Changes the resolution to be used for this representation
       * \param[in] resolution resolution vector.
       *
       */
      void setResolution(const NmVector3 &resolution);

      /** \brief Set the external settings to be used on actor creation
       * \param[in] settings representation settings.
       *
       */
      void setSettings(const RepresentationState &settings);

      /** \brief Add sources representations to be update
       * \param[in] sources list of items to update the actors.
       *
       */
      void updateRepresentations(ViewItemAdapterList sources);

      /** \brief Add sources representations to update the colors.
       * \param[in] sources list of items to update the actors' colors.
       *
       */
      void updateRepresentationColors(const ViewItemAdapterList &sources);

      /** \brief Sets the execution TimeStamp of the task
       * \param[in] frame display frame parameters.
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

      /** \brief Returns the list of items whose representation occupies the given point.
       * \param[in] point pick point.
       * \param[in] actor if not null is an actor picked in the view to check it's belonging to this updater.
       *
       */
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

      /** \brief Returns the current pipeline for the given item.
       * \param[in] item view item.
       *
       */
      RepresentationPipelineSPtr sourcePipeline(ViewItemAdapterPtr item) const;

      /** \brief Returns the item that corresponds with the given actor.
       * \param[in] actor vtk actor.
       *
       */
      ViewItemAdapterPtr findActorItem(vtkProp *actor) const;

      /** \brief Adds the sources for being updated in the next execution.
       * \param[in] sources items to update.
       * \param[in] createActors true to create new actors and false to update the previous actor's colors.
       *
       */
      void updateRepresentations(ViewItemAdapterList sources, const bool createActors);

      /** \brief Adds a source to be updated to an update list.
       *
       */
      static void addUpdateRequest(UpdateRequestList &list, ViewItemAdapterPtr item, const bool createActors);

      /** \brief Removes a source to be updated to an update list.
       *
       */
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
