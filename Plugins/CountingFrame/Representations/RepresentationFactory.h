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

#ifndef ESPINA_CF_REPRESENTATION_FACTORY_H
#define ESPINA_CF_REPRESENTATION_FACTORY_H

#include "CountingFramePlugin_Export.h"

// ESPINA
#include <Support/Representations/RepresentationFactory.h>

// Plugin
#include "CountingFrameManager.h"

namespace ESPINA
{
  namespace CF
  {
    class CountingFramePlugin_EXPORT RepresentationFactory
    : public ESPINA::RepresentationFactory
    {
    public:
      explicit RepresentationFactory(CountingFrameManager *manager);

    private:
      virtual Representation doCreateRepresentation(Support::Context &context, ViewTypeFlags supportedViews) const;

      CountingFrameManager *m_manager;
    };
  }
}

#endif // ESPINA_CF_REPRESENTATION_FACTORY_H
