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

#include <Core/Utils/NmVector3.h>
#include <Core/EspinaTypes.h>
#include "RepresentationState.h"

#include <QString>
#include <QList>

#include <memory>

#include <vtkSmartPointer.h>

class vtkProp;

namespace ESPINA {

class ViewItemAdapter;

  /** \class RepresentationPipeline
   *
   * This representation pipeline settings are ThreadSafe
   */
  class RepresentationPipeline
  {
  public:
    using Type      = QString;
    using VTKActor  = vtkSmartPointer<vtkProp>;
    using ActorList = QList<VTKActor>;
    using Actors    = QMap<ViewItemAdapter*, ActorList>;

  public:
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

    virtual RepresentationState representationState(const ViewItemAdapter     *item,
                                                    const RepresentationState &settings) = 0;

    /** \brief Create the actors for the view item with the given state
     *
     *  NOTE: Must be reentrant
     */
    virtual RepresentationPipeline::ActorList createActors(const ViewItemAdapter     *item,
                                                           const RepresentationState &state) = 0;

    virtual bool pick(ViewItemAdapter *item, const NmVector3 &point) const = 0;

  protected:
    explicit RepresentationPipeline(Type type);

    void setType(const Type &type)
    { m_type = type; }

  private:
    Type m_type;
  };

  using RepresentationPipelineSPtr  = std::shared_ptr<RepresentationPipeline>;
  using RepresentationPipelineSList = QList<RepresentationPipelineSPtr>;
}

#endif // ESPINA_REPRESENTATIONPIPELINE_H
