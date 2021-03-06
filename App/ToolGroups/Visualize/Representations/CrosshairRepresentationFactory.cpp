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
#include <App/ToolGroups/Visualize/Representations/CrosshairRepresentationFactory.h>
#include <App/ToolGroups/Visualize/Representations/Switches/CrosshairSwitch.h>
#include <GUI/Representations/Managers/CrosshairManager.h>
#include <GUI/View/ViewTypeFlags.h>

using namespace ESPINA;
using namespace ESPINA::GUI::Representations::Managers;

//----------------------------------------------------------------------------
Representation CrosshairRepresentationFactory::doCreateRepresentation(Support::Context &context, ViewTypeFlags supportedViews) const
{
  Representation representation;

  representation.Group = "Crosshair";

  if (supportedViews.testFlag(ESPINA::VIEW_2D))
  {
    createCrosshair2D(representation, context);
  }

  if (supportedViews.testFlag(ESPINA::VIEW_3D))
  {
    createCrosshair3D(representation, context);
  }

  return representation;
}

//----------------------------------------------------------------------------
void CrosshairRepresentationFactory::createCrosshair2D(Representation &representation, Support::Context &context) const
{
  createCrosshair(":espina/display_crosshairs.svg",
                  QObject::tr("Display Crosshairs"),
                  representation,
                  ViewType::VIEW_2D,
                  context);
}

//----------------------------------------------------------------------------
void CrosshairRepresentationFactory::createCrosshair3D(Representation &representation, Support::Context &context) const
{
  createCrosshair(":espina/display_crosshairs.svg",
                  QObject::tr("Display Crosshairs"),
                  representation,
                  ViewType::VIEW_3D,
                  context);
}

//----------------------------------------------------------------------------
void CrosshairRepresentationFactory::createCrosshair(const QString   &icon,
                                                     const QString   &description,
                                                     Representation  &representation,
                                                     ViewTypeFlags    flags,
                                                     Support::Context &context) const
{
  auto crossManager = std::make_shared<CrosshairManager>(flags);

  crossManager->setName(QObject::tr("DisplayCrosshair"));
  crossManager->setIcon(QIcon(icon));
  crossManager->setDescription(description);

  auto crossSwitch  = std::make_shared<CrosshairSwitch>(crossManager, flags, context);
  crossSwitch->setOrder("0", "2-Display");

  if(flags.testFlag(ViewType::VIEW_2D))
  {
    crossSwitch->setShortcut(Qt::Key_C);
  }

  representation.Managers << crossManager;
  representation.Switches << crossSwitch;
}
