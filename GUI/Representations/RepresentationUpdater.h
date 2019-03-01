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

      /** \brief Returns the latest actors computed by the task
       *
       */
      RepresentationPipeline::Actors actors() const;

    signals:
      void actorsReady(const GUI::Representations::FrameCSPtr frame, RepresentationPipeline::Actors actors);

    protected:
      virtual void run();

    public:
      using UpdateRequest     = QPair<ViewItemAdapterPtr, bool>;
      using UpdateRequestList = QList<UpdateRequest>;

    private:
      /** \brief Returns the current pipeline for the given item.
       * \param[in] item view item.
       *
       */
      RepresentationPipelineSPtr sourcePipeline(ViewItemAdapterPtr item) const;

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

    protected:
      GUI::Representations::FrameCSPtr m_frame;            /** frame of the actors.                                       */
      RepresentationPipelineSPtr       m_pipeline;         /** actor creation pipeline and pick resolver.                 */

      UpdateRequestList                m_requestedSources; /** list of requested to update sources.                       */
      UpdateRequestList                m_sources;          /** items to create or update.                                 */
      UpdateRequestList               *m_updateList;       /** pointer to the list to update during the updater run.      */

      RepresentationState              m_settings;         /** representation's settings.                                 */
      RepresentationPipeline::Actors   m_actors;           /** list of actors of the frame.                               */

      mutable QReadWriteLock           m_dataLock;         /** protects the execution data.                               */
      mutable bool                     m_needsUpdate;      /** true if the updater can effectively run, false otherwise.  */
  };

  using RepresentationUpdaterSPtr  = std::shared_ptr<RepresentationUpdater>;
  using RepresentationUpdaterSList = QList<RepresentationUpdaterSPtr>;
}

#endif // ESPINA_REPRESENTATIONUPDATER_H
