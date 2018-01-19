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

// ESPINA
#include "EspinaMainWindow.h"
#include <Core/Utils/EspinaException.h>

// Qt
#include <QApplication>
#include <QPluginLoader>
#include <QTranslator>
#include <QSharedMemory>
#include <QMessageBox>
#include <QDebug>

using namespace ESPINA;
using namespace ESPINA::Core::Utils;

int main(int argc, char **argv)
{
  QApplication app(argc, argv);

  // allow only one instance
//  QSharedMemory guard;
//  guard.setKey("EspINA");
//
//  if (!guard.create(1))
//  {
//    QMessageBox msgBox;
//    msgBox.setWindowIcon(QIcon(":/espina/espina.svg"));
//    msgBox.setWindowTitle("EspINA");
//    msgBox.setIcon(QMessageBox::Warning);
//    msgBox.setText("EspINA is already running!");
//    msgBox.setStandardButtons(QMessageBox::Ok);
//    msgBox.exec();
//    exit(0);
//  }

  QTranslator translator;
  translator.load("espina_es");
  app.installTranslator(&translator);

  QDir pluginsDir = QDir(app.applicationDirPath());
  pluginsDir.cd("plugins");

  qDebug() << "Loading Plugins from path: " << pluginsDir.absolutePath();

  installExceptionHandler();
  installSignalHandler();
  installVTKErrorLogger();

  QList<QPluginLoader *> loaders;
  QList<QObject *>       plugins;

  qDebug() << "Loading Plugins: ";
  for(QString fileName : pluginsDir.entryList(QDir::Files))
  {
    auto loader = new QPluginLoader(pluginsDir.absoluteFilePath(fileName));
    auto plugin = loader->instance();

    if (plugin)
    {
      qDebug() << "Found plugin: " << fileName;
      plugins << plugin;
      loaders << loader;
    }
    else
    {
      // DO NOT DELETE, THIS IS TO DEBUG PLUGINS
      qDebug() << "ERROR:" << fileName << "not loaded. Description:" << loader->errorString();
      delete loader;
    }
  }

  int res = 0;
  {
    EspinaMainWindow espina(plugins);
    espina.setWindowIcon(QIcon(":/espina/espina.svg"));
    espina.show();

    if (argc > 1)
    {
      QStringList filenames;
      for (int i = 1; i < argc; ++i)
      {
        filenames << QString(argv[i]);
      }

      espina.openAnalysis(filenames);
    }

    res = app.exec();
  }

  // plugins must be deleted after EspINA application has been destroyed.
  for(auto plugin: loaders)
  {
    plugin->unload();
    delete plugin;
  }

  qDebug() << "ESPINA exited with value" << res;
  return res;
}
