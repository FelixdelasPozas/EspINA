 /*

 Copyright (C) 2016 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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

// Project
#include <ToolsDev/fakeStacks/FakeStacks.h>
#include <QApplication>
#include <QDebug>
#include <QIcon>
#include <QMessageBox>
#include <QtCore/QtCore>

int main(int argc, char *argv[])
{
  QApplication app(argc, argv);

  FakeStacks mainDialog;
  mainDialog.show();

  auto result = app.exec();

  return result;
}
