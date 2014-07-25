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

#include "GUI/Representations/Representation.h"
#include <Core/MultiTasking/Scheduler.h>

namespace ESPINA
{
  using RepresentationTypeList = QList<Representation::Type>;

  class EspinaGUI_EXPORT RepresentationFactory
  {
  public:
    explicit RepresentationFactory(SchedulerSPtr scheduler)
    { m_scheduler = scheduler; };

    virtual ~RepresentationFactory() {};

    virtual RepresentationTypeList representations() const = 0;

    virtual RepresentationSPtr createRepresentation(OutputSPtr output, Representation::Type representation) = 0;

  protected:
    SchedulerSPtr m_scheduler;
  };

  using RepresentationFactorySPtr = std::shared_ptr<RepresentationFactory>;
}

#endif // ESPINA_REPRESENTATION_FACTORY_H
