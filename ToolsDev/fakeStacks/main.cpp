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

  // allow only one instance
//  QSharedMemory guard;
//  guard.setKey("EspINA_FakeStacks");
//
//  if (!guard.create(1))
//  {
//    QMessageBox msgBox;
//    msgBox.setWindowIcon(QIcon(":/Tools/application.ico"));
//    msgBox.setIcon(QMessageBox::Warning);
//    msgBox.setText("EspINA Spacing Changer is already running!");
//    msgBox.setStandardButtons(QMessageBox::Ok);
//    msgBox.exec();
//    guard.detach();
//    exit(0);
//  }

  FakeStacks mainDialog;
  mainDialog.show();

  auto result = app.exec();
//  guard.detach();

  return result;
}
