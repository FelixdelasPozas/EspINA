/*

    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>

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

#include "Tool.h"

using namespace ESPINA;

//----------------------------------------------------------------------------
Tool::Tool()
: m_enabled{false}
{

}

//----------------------------------------------------------------------------
QPushButton *Tool::createToolButton(const QIcon &icon, const QString &tooltip)
{
  auto button = new QPushButton();

  button->setIcon(icon);
  button->setIconSize(QSize(22, 22));
  button->setMaximumSize(30, 30);
  button->setFlat(true);
  button->setToolTip(tooltip);

  return button;
}

//----------------------------------------------------------------------------
void Tool::setEnabled(bool value)
{
  if (m_enabled != value)
  {
    onToolEnabled(value);
  }

  m_enabled = value;
}

//----------------------------------------------------------------------------
bool Tool::isEnabled() const
{
  return m_enabled;
}