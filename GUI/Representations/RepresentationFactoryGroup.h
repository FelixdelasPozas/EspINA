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

#ifndef ESPINA_REPRESENTATION_FACTORY_GROUP_H
#define ESPINA_REPRESENTATION_FACTORY_GROUP_H

// ESPINA
#include <GUI/Representations/RepresentationFactory.h>

// Qt
#include <QMap>

namespace ESPINA
{
  class EspinaGUI_EXPORT RepresentationFactoryGroup
  : public RepresentationFactory
  {
  public:
    struct Representation_Already_Provided_Exception{};

  	/** brief RepresentationFactoryGroup class constructor.
  	 * \param[in] scheduler, scheduler smart pointer.
  	 *
  	 */
    explicit RepresentationFactoryGroup(SchedulerSPtr scheduler)
    : RepresentationFactory{scheduler}
    {};

  	/** brief RepresentationFactoryGroup class virtual destructor.
  	 *
  	 */
    virtual ~RepresentationFactoryGroup()
    {};

  	/** brief Adds a factory to the factory group.
  	 * \param[in] factory, RepresentationFactor smart pointer.
  	 *
  	 */
    void addRepresentationFactory(RepresentationFactorySPtr factory);

  	/** brief Returns the list of representations this factory can create.
  	 *
  	 */
    virtual RepresentationTypeList representations() const;

  	/** brief Creates and returns a representation of the given type for the given output.
  	 * \param[in] output, output smart pointer.
  	 * \param[in] type, representation type.
  	 *
  	 */
    virtual ESPINA::RepresentationSPtr createRepresentation(OutputSPtr output, Representation::Type type);

  private:
    QMap<Representation::Type, RepresentationFactorySPtr> m_factories;
  };

  using RepresentationFactoryGroupSPtr = std::shared_ptr<RepresentationFactoryGroup>;
}

#endif // ESPINA_REPRESENTATION_FACTORY_GROUP_H
