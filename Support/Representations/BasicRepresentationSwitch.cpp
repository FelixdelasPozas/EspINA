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

using namespace ESPINA;

//----------------------------------------------------------------------------
BasicRepresentationSwitch::BasicRepresentationSwitch(RepresentationManagerSPtr manager,
                                                     ViewTypeFlags             supportedViews)
: m_manager(manager)
, m_flags(supportedViews)
{
  setActive(m_manager->representationsVisibility());
}

//----------------------------------------------------------------------------
ESPINA::ViewTypeFlags BasicRepresentationSwitch::supportedViews()
{
  return m_flags;
}

//----------------------------------------------------------------------------
QWidget* BasicRepresentationSwitch::widget()
{
  auto widget = Tool::createToolButton(m_manager->icon(), m_manager->description());

  widget->setCheckable(true);
  widget->setChecked(isActive());

  connect(widget, SIGNAL(toggled(bool)),
          this,   SLOT(onButtonToggled(bool)));

  return widget;
}

//----------------------------------------------------------------------------
void BasicRepresentationSwitch::showRepresentations()
{
  m_manager->show();
}

//----------------------------------------------------------------------------
void BasicRepresentationSwitch::hideRepresentations()
{
  m_manager->hide();
}

//----------------------------------------------------------------------------
void BasicRepresentationSwitch::onButtonToggled(bool active)
{
  setActive(active);
}