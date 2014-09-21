/*
 *
 * Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 * This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
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

#ifndef ESPINA_REPRESENTATION_FACTORY_H
#define ESPINA_REPRESENTATION_FACTORY_H

#include "GUI/EspinaGUI_Export.h"

// ESPINA
#include "GUI/Representations/Representation.h"
#include <Core/MultiTasking/Scheduler.h>

namespace ESPINA
{
  using RepresentationTypeList = QList<Representation::Type>;

  class EspinaGUI_EXPORT RepresentationFactory
  {
  public:
  	/** brief RepresentationFactory class constructor.
  	 * \param[in] scheduler, scheduler smart pointer.
  	 *
  	 */
    explicit RepresentationFactory(SchedulerSPtr scheduler)
    { m_scheduler = scheduler; };

    /** brief RepresentationFactory class virtual destructor.
     *
     */
    virtual ~RepresentationFactory()
    {};

    /** brief Returns the list of representation types this factory can create.
     *
     */
    virtual RepresentationTypeList representations() const = 0;

    /** brief Creates and returns a representation of the given type for the given output.
     * \param[in] output, output smart pointer.
     * \param[in] type, representation type.
     *
     */
    virtual RepresentationSPtr createRepresentation(OutputSPtr output, Representation::Type representation) = 0;

  protected:
    SchedulerSPtr m_scheduler;
  };

  using RepresentationFactorySPtr = std::shared_ptr<RepresentationFactory>;
}

#endif // ESPINA_REPRESENTATION_FACTORY_H
