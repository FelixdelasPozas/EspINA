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
   *
   * This representation pipeline settings are ThreadSafe
   */
  class EspinaGUI_EXPORT RepresentationPipeline
  {
    public:
      using Type      = QString;
      using VTKActor  = vtkSmartPointer<vtkProp>;
      using ActorList = QList<VTKActor>;

      struct ActorsData
      {
          QMap<ViewItemAdapter*, ActorList> actors; /** map of item-item's actors. */
          QMutex                            lock;   /** access lock.               */
      };

      using Actors = std::shared_ptr<ActorsData>;

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

      /** \brief Returns the serialized settings of the representation
       *
       */
      virtual QString serializeSettings();

      /** \brief Restores the settings for the representation
       * \param[in] settings serialization
       *
       */
      virtual void restoreSettings(QString settings);

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
      explicit RepresentationPipeline(Type type);

      void setType(const Type &type)
      { m_type = type; }

    private:
      Type m_type; /** type of the pipeline. */
  };

  using RepresentationPipelineSPtr  = std::shared_ptr<RepresentationPipeline>;
  using RepresentationPipelineSList = QList<RepresentationPipelineSPtr>;
}

#endif // ESPINA_REPRESENTATIONPIPELINE_H
