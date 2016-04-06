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

// Qt
#include <QApplication>
#include <QPluginLoader>
#include <QTranslator>
#include <QDebug>

using namespace ESPINA;

int main(int argc, char **argv)
{
  QApplication app(argc, argv);
  app.setWindowIcon(QIcon(":/espina/espina.svg"));

  QTranslator translator;
  translator.load("espina_es");
  app.installTranslator(&translator);

  QDir pluginsDir = QDir(app.applicationDirPath());
  qDebug() << "Loading Plugins from path: " << pluginsDir.absolutePath();

  #if defined(Q_OS_MAC)
    if (pluginsDir.dirName() == "MacOS")
    {
      pluginsDir.cdUp();
      pluginsDir.cdUp();
      pluginsDir.cdUp();
    }
  #endif

  pluginsDir.cd("plugins");

  QList<QPluginLoader *> loaders;
  QList<QObject *>       plugins;

  qDebug() << "Loading Plugins: ";
  for(QString fileName : pluginsDir.entryList(QDir::Files))
  {
    auto loader = new QPluginLoader(pluginsDir.absoluteFilePath(fileName));
    auto plugin = loader->instance();

    if (plugin)
    {
      qDebug() << "Found plugin " << fileName;
      plugins << plugin;
      loaders << loader;
    }
    else
    {
      // DO NOT DELETE, THIS IS TO DEBUG PLUGINS
      qDebug() << fileName << "not loaded -> Error:" << loader->errorString();
      delete loader;
    }
  }

  int res = 0;
  {
    EspinaMainWindow espina(plugins);
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

  for(auto plugin: loaders)
  {
    plugin->unload();
    delete plugin;
  }

  qDebug() << "ESPINA exited with value" << res;
  return res;
}
