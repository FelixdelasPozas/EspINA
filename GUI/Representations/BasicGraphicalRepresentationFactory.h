/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2013  Jorge Peña Pastor <jpena@cesvima.upm.es>
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

#ifndef ESPINA_BASICGRAPHICALREPRESENTATIONFACTORY_H
#define ESPINA_BASICGRAPHICALREPRESENTATIONFACTORY_H

#include <Core/Model/Filter.h>
#include <GUI/Representations/GraphicalRepresentationFactory.h>

namespace EspINA
{
  class BasicGraphicalRepresentationFactory 
  : public GraphicalRepresentationFactory
  {
  public:
    virtual void createGraphicalRepresentations(ChannelOutputSPtr output);

    virtual void createGraphicalRepresentations(SegmentationOutputSPtr output);
  };

  void SetBasicGraphicalRepresentationFactory(Filter    *filter);
  void SetBasicGraphicalRepresentationFactory(FilterSPtr filter);
}

#endif // ESPINA_BASICGRAPHICALREPRESENTATIONFACTORY_H
