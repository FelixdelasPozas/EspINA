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
#include <GUI/Representations/Managers/CrosshairManager.h>
#include <GUI/View/ViewTypeFlags.h>
#include <Support/Representations/BasicRepresentationSwitch.h>
#include <ToolGroups/Visualize/Representations/CrosshairRepresentationFactory.h>

using namespace ESPINA;
using ESPINA::GUI::Representations::Managers::CrosshairManager;

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

  representation.Icon        = QIcon(":espina/crosshairs2D_switch.svg");
  representation.Description = QObject::tr("Shows/Hides the crosshair");

  return representation;
}

//----------------------------------------------------------------------------
void CrosshairRepresentationFactory::createCrosshair2D(Representation &representation, Support::Context &context)
{
  createCrosshair(":espina/crosshairs2D_switch.svg",
                  QObject::tr("Shows/Hides the crosshair in the 2D views."),
                  representation,
                  ViewType::VIEW_2D,
                  context);
}

//----------------------------------------------------------------------------
void CrosshairRepresentationFactory::createCrosshair3D(Representation &representation, Support::Context &context)
{
  createCrosshair(":espina/crosshairs3D_switch.svg",
                  QObject::tr("Shows/Hides the crosshair in the 3D view."),
                  representation,
                  ViewType::VIEW_3D,
                  context);
}

//----------------------------------------------------------------------------
void CrosshairRepresentationFactory::createCrosshair(const QString   &icon,
                                                     const QString   &description,
                                                     Representation  &representation,
                                                     ViewTypeFlags    flags,
                                                     Support::Context &context)
{
  auto crossManager = std::make_shared<CrosshairManager>(flags);
  auto crossSwitch  = std::make_shared<BasicRepresentationSwitch>(crossManager, flags, context.timer());

  crossManager->setName(QObject::tr("Crosshair"));
  crossManager->setIcon(QIcon(icon));
  crossManager->setDescription(description);

  representation.Managers << crossManager;
  representation.Switches << crossSwitch;
}
