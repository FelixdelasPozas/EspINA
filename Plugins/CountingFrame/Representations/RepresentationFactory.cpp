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
#include "Representations/RepresentationSwitch.h"
#include <Support/Representations/BasicRepresentationSwitch.h>

using namespace ESPINA;
using namespace ESPINA::CF;

//------------------------------------------------------------------------
CF::RepresentationFactory::RepresentationFactory(CountingFrameManager *manager)
: m_manager{manager}
{
}

//------------------------------------------------------------------------
Representation CF::RepresentationFactory::doCreateRepresentation(Support::Context &context, ViewTypeFlags supportedViews) const
{
  Representation representation;

  if (supportedViews.testFlag(VIEW_2D))
  {
    auto manager2D = std::make_shared<RepresentationManager2D>(m_manager, ViewType::VIEW_2D);

    manager2D->setName(QObject::tr("Counting Frame"));
    manager2D->setIcon(QIcon(":cf-switch2D.svg"));
    manager2D->setDescription(QObject::tr("Display Stereological Counting Frame"));

    auto switch2D = std::make_shared<BasicRepresentationSwitch>("CF2DSwitch", manager2D, ViewType::VIEW_2D, context);
    switch2D->setChecked(true);
    switch2D->setOrder("1", "2-Display");

    representation.Managers << manager2D;
    representation.Switches << switch2D;
  }

  if (supportedViews.testFlag(VIEW_3D))
  {
    auto manager3D = std::make_shared<RepresentationManager3D>(m_manager, ViewType::VIEW_3D);

    manager3D->setName(QObject::tr("Counting Frame"));
    manager3D->setIcon(QIcon(":cf-switch3D.svg"));
    manager3D->setDescription(QObject::tr("Display Stereological Counting Frame"));

    auto switch3D = std::make_shared<CFRepresentationSwitch>(manager3D, context);
    switch3D->setOrder("1", "2-Display");

    representation.Managers << manager3D;
    representation.Switches << switch3D;
  }

  return representation;
}
