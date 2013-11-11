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

#ifndef ESPINA_BASIC_REPRESENTATION_FACTORY_H
#define ESPINA_BASIC_REPRESENTATION_FACTORY_H

#include "EspinaGUI_Export.h"
#include "GUI/Representations/RepresentationFactory.h"
#include <GUI/Model/FilterAdapter.h>


namespace EspINA
{
  class EspinaGUI_EXPORT BasicChannelRepresentationFactory
  : public RepresentationFactory
  {
  public:
    virtual ~BasicChannelRepresentationFactory() {};

    virtual RepresentationTypeList representations() const;

    virtual RepresentationSPtr createRepresentation(OutputSPtr output, Representation::Type type);
  };

  class EspinaGUI_EXPORT BasicSegmentationRepresentationFactory
  : public RepresentationFactory
  {
  public:
    virtual ~BasicSegmentationRepresentationFactory(){}

    virtual RepresentationTypeList representations() const;

    virtual RepresentationSPtr createRepresentation(OutputSPtr output, Representation::Type type);
  };

  //void EspinaGUI_EXPORT SetBasicRepresentationFactory(FilterAdapterSPtr filter);
}

#endif // ESPINA_BASIC_REPRESENTATION_FACTORY_H
