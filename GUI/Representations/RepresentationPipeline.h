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

#ifndef ESPINA_REPRESENTATION_PIPELINE_H
#define ESPINA_REPRESENTATION_PIPELINE_H

#include <GUI/EspinaGUI_Export.h>

// ESPINA
#include <Core/Types.h>
#include <Core/Utils/Vector3.hxx>
#include <Core/Utils/Locker.h>
#include <GUI/Types.h>
#include "RepresentationState.h"

// Qt
#include <QString>
#include <QList>
#include <QMutex>

// C++
#include <memory>

// VTK
#include <vtkSmartPointer.h>

class vtkProp;

namespace ESPINA
{
  /** \class RepresentationPipeline
   * \brief Base class that implements the API for creating the actors for view items.
   *
   * This representation pipeline settings are ThreadSafe if using the mutex to access data.
   */
  class EspinaGUI_EXPORT RepresentationPipeline
  {
    public:
      using Type      = QString;
      using VTKActor  = vtkSmartPointer<vtkProp>;
      using ActorList = QList<VTKActor>;
      using ActorsMap = QMap<ViewItemAdapter*, ActorList>;
      class ActorsLocker;

      struct ActorsData
      {
        private:
          friend class ActorsLocker;

          ActorsMap actors; /** map of item<->item's actors. */
          QMutex    lock;   /** access lock.               */
      };

      using Actors = std::shared_ptr<ActorsData>;

      /** \class ActorsLocker
       * \brief Implements a class to lock actors given as parameter.
       *
       */
      class ActorsLocker
      : public Core::Utils::MutexLocker
      {
        public:
          /** \brief ActorsLocker class constructor.
           * \param[in] data Actors object.
           * \param[in] tryLock true to try the lock instead of locking right away and false otherwise.
           *
           */
          explicit ActorsLocker(Actors data, bool tryLock = false)
          : Core::Utils::MutexLocker(data->lock, tryLock)
          , m_data                  {data}
          {};

          ActorsMap& get()
          { return m_data->actors; }

        private:
          Actors m_data; /** actors data. */
      };

    public:
      /** \brief RepresentationPipeline class virtual destructor.
       *
       */
      virtual ~RepresentationPipeline()
      {}

      /** \brief Returns the type of the representation pipeline
       *
       */
      Type type() const
      { return m_type; }

      /** \brief Returns the representation settings for the given item.
       * \param[in] item view item pointer.
       * \param[in] settings pipeline settings.
       *
       */
      virtual RepresentationState representationState(ConstViewItemAdapterPtr   item,
                                                      const RepresentationState &settings) = 0;

      /** \brief Returns true if the point is inside the item representation.
       * \param[in] item void item pointer to check.
       * \param[in] point picked point.
       *
       */
      virtual bool pick(ConstViewItemAdapterPtr item, const NmVector3 &point) const = 0;

      /** \brief Create the actors for the view item with the given state.
       * \param[in] item view item pointer.
       * \param[in] state item's representation settings.
       *
       *  NOTE: This function must be reentrant
       */
      virtual RepresentationPipeline::ActorList createActors(ConstViewItemAdapterPtr  item,
                                                             const RepresentationState &state) = 0;

      /** \brief Update the color of the representation actors
       * \param[in] actors list of previous actors to modify.
       * \param[in] item view item pointer.
       * \param[in] state item's representation settings.
       *
       *  NOTE: This function must be reentrant
       */
      virtual void updateColors(RepresentationPipeline::ActorList &actors,
                                ConstViewItemAdapterPtr           item,
                                const RepresentationState         &state) = 0;

    protected:
      /** \brief RepresentationPipeline constructor.
       * \param[in] type type of the pipeline.
       *
       */
      explicit RepresentationPipeline(Type type)
      : m_type{type}
      {}

      /** \brief Sets the type of the pipeline.
       * \param[in] type type of the pipeline.
       *
       */
      void setType(const Type &type)
      { m_type = type; }

    private:
      Type m_type; /** type of the pipeline (identifier). */
  };

  using RepresentationPipelineSPtr  = std::shared_ptr<RepresentationPipeline>;
  using RepresentationPipelineSList = QList<RepresentationPipelineSPtr>;
}

#endif // ESPINA_REPRESENTATIONPIPELINE_H
