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

#include "Panel.h"

#include <QPushButton>
#include <QApplication>

using namespace ESPINA;

//------------------------------------------------------------------------
Panel::Panel(Support::Context &context, QWidget* parent)
: QDockWidget(parent)
, WithContext(context)
{
}

//------------------------------------------------------------------------
Panel::Panel(const QString& title, Support::Context &context, QWidget* parent, Qt::WindowFlags flags)
: QDockWidget(title, parent, flags)
, WithContext(context)
{
}

//------------------------------------------------------------------------
QPushButton *Panel::createDockButton(const QString &icon, const QString &tooltip)
{
  auto dockButton = new QPushButton();

  dockButton->setIcon(QIcon(icon));
  dockButton->setIconSize(QSize(22,22));
  dockButton->setBaseSize(32, 32);
  dockButton->setMaximumSize(32, 32);
  dockButton->setMinimumSize(32, 32);
  dockButton->setFlat(true);
  dockButton->setToolTip(tooltip);

  return dockButton;
}

//------------------------------------------------------------------------
void Panel::showEvent(QShowEvent *event)
{
  QWidget::showEvent(event);

  emit dockShown(true);
}

//------------------------------------------------------------------------
void Panel::hideEvent(QHideEvent *event)
{
  QWidget::hideEvent(event);

  if (!getContext().isMinimized())
  {
    emit dockShown(false);
  }
}
