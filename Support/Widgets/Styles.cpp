/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2015  <copyright holder> <email>
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

#include "Styles.h"
#include <QWidget>
#include <QPushButton>

using namespace ESPINA::Support::Widgets;

//-----------------------------------------------------------------------------
void Styles::setNestedStyle(QWidget *widget)
{
  widget->setObjectName("NestedWidget");
  widget->setStyleSheet(
    "#NestedWidget {"
    "  margin-top: 4px;"
    "  margin-bottom: 4px;"
    "  padding-right: 30px;"
    "  border-radius: 5px;"
    "  background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, "
                         "stop:   0 rgba(0,0,0,20),"
                         "stop: 0.5 rgba(0,0,0,50),"
                         "stop:   1 rgba(0,0,0,20))}"
  );
}

//-----------------------------------------------------------------------------
QPushButton* Styles::createToolButton(const QString& icon, const QString& tooltip)
{
  auto button = new QPushButton();

  button->setIcon(QIcon(icon));
  button->setIconSize(QSize(22, 22));
  button->setMaximumSize(30, 30);
  button->setFlat(true);
  button->setToolTip(tooltip);

  return button;
}
