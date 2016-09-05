/*
 *    Copyright (C) 2015  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 *    This file is part of ESPINA.
 *
 *    ESPINA is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SUPPORT_CONTEXT_FACTORIES_H
#define SUPPORT_CONTEXT_FACTORIES_H

#include <Support/EspinaSupport_Export.h>

// ESPINA
#include <Core/Factory/FilterFactory.h>

namespace ESPINA
{
  namespace Support
  {
    class Context;

    namespace ContextFactories
    {
      void EspinaSupport_EXPORT registerFilterFactory(Context &context, FilterFactorySPtr factory);
    };
  }
}

#endif // SUPPORT_CONTEXT_FACTORIES_H
