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

#include "BasicRepresentationSwitch.h"

#include <Support/Widgets/Tool.h>
#include <GUI/Widgets/Styles.h>
#include <QPushButton>

using namespace ESPINA;
using namespace ESPINA::GUI::Representations;
using namespace ESPINA::GUI::Widgets;

//----------------------------------------------------------------------------
BasicRepresentationSwitch::BasicRepresentationSwitch(RepresentationManagerSPtr manager,
                                                     ViewTypeFlags             supportedViews,
                                                     Timer                    &timer,
                                                     Support::Context         &context)
: RepresentationSwitch(manager->icon(), manager->description(), timer, context)
, m_manager(manager)
, m_flags(supportedViews)
{
  setChecked(m_manager->representationsVisibility());
}

//----------------------------------------------------------------------------
ESPINA::ViewTypeFlags BasicRepresentationSwitch::supportedViews()
{
  return m_flags;
}

//----------------------------------------------------------------------------
void BasicRepresentationSwitch::showRepresentations(TimeStamp t)
{
  m_manager->show(t);
}

//----------------------------------------------------------------------------
void BasicRepresentationSwitch::hideRepresentations(TimeStamp t)
{
  m_manager->hide(t);
}