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

#ifndef ESPINA_REPRESENTATION_FACTORY_GROUP_H
#define ESPINA_REPRESENTATION_FACTORY_GROUP_H

#include <GUI/Representations/RepresentationFactory.h>
#include <QMap>

namespace EspINA {

  class EspinaGUI_EXPORT RepresentationFactoryGroup 
  : public RepresentationFactory
  {
  public:
    explicit RepresentationFactoryGroup(SchedulerSPtr scheduler)
    : RepresentationFactory(scheduler)
    {};

    virtual ~RepresentationFactoryGroup() {};

    struct Representation_Already_Provided_Exception{};
  public:
    void addRepresentationFactory(RepresentationFactorySPtr factory);

    virtual RepresentationTypeList representations() const;

    virtual EspINA::RepresentationSPtr createRepresentation(OutputSPtr output, Representation::Type type);

  private:
    QMap<Representation::Type, RepresentationFactorySPtr> m_factories;
  };

  using RepresentationFactoryGroupSPtr = std::shared_ptr<RepresentationFactoryGroup>;
}

#endif // ESPINA_REPRESENTATION_FACTORY_GROUP_H
