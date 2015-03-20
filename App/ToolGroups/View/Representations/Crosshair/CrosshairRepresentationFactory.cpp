/*

 Copyright (C) 2015 Félix de las Pozas Álvarez <fpozas@cesvima.upm.es>

 This file is part of ESPINA.

 ESPINA is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

// ESPINA
#include <GUI/Representations/CrosshairManager.h>
#include <GUI/View/ViewTypeFlags.h>
#include <Support/Representations/BasicRepresentationSwitch.h>
#include <ToolGroups/View/Representations/Crosshair/CrosshairRepresentationFactory.h>

namespace ESPINA
{
  //----------------------------------------------------------------------------
  Representation CrosshairRepresentationFactory::createRepresentation(ColorEngineSPtr colorEngine) const
  {
    Representation representation;

    auto crossManager = std::make_shared<CrosshairManager>();
    auto crossSwitch  = std::make_shared<BasicRepresentationSwitch>(crossManager, ViewType::VIEW_2D|ViewType::VIEW_3D);

    // representation.Group;
    // representation.Pools;
    representation.Managers   << crossManager;
    representation.Switches   << crossSwitch;
    representation.Icon        = QIcon(":espina/crosshair_planes.svg");
    representation.Description = QObject::tr("Shows/Hides the crosshair");

    return representation;
  }
} // namespace ESPINA
