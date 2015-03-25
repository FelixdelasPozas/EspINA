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

#include "RepresentationFactory.h"

#include "Representations/RepresentationManager2D.h"
#include "Representations/RepresentationManager3D.h"
#include <Support/Representations/BasicRepresentationSwitch.h>

namespace ESPINA {
  namespace CF {
    //------------------------------------------------------------------------
    RepresentationFactory::RepresentationFactory(CountingFrameManager &manager)
    : m_manager(manager)
    {
    }

    //------------------------------------------------------------------------
    Representation RepresentationFactory::createRepresentation(ColorEngineSPtr colorEngine) const
    {
      Representation representation;

      auto manager2D = std::make_shared<RepresentationManager2D>(m_manager, ViewType::VIEW_2D);
      auto switch2D  = std::make_shared<BasicRepresentationSwitch>(manager2D, ViewType::VIEW_2D);

      auto manager3D = std::make_shared<RepresentationManager3D>(m_manager, ViewType::VIEW_3D);
      auto switch3D  = std::make_shared<BasicRepresentationSwitch>(manager3D, ViewType::VIEW_3D);

      manager2D->setName(QObject::tr("Counting Frame"));
      manager2D->setIcon(QIcon(":cf-switch2D.svg"));

      switch2D->setActive(true);

      manager3D->setName(QObject::tr("Counting Frame"));
      manager3D->setIcon(QIcon(":cf-switch3D.svg"));

      representation.Group = "CountingFrame";
      representation.Managers << manager2D << manager3D;
      representation.Switches << switch2D << switch3D;
      representation.Icon = QIcon(":cf-representation.svg");
      representation.Description = "Show Counting Frames";

      return representation;
    }
  }
}
