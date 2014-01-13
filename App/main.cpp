/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  Jorge Peña Pastor <jpena@cesvima.upm.es>

    This program is free software: you can redistribute it and/or modify
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

#include <QApplication>
#include <QPluginLoader>
#include <QTranslator>

#include "EspinaMainWindow.h"

#include <Core/MultiTasking/Scheduler.h>
#include <Core/Analysis/Analysis.h>

using namespace EspINA;

int main(int argc, char **argv)
{
  QApplication app(argc, argv);
  app.setWindowIcon(QIcon(":/espina/espina.svg"));

  QTranslator translator;
  translator.load("espina_es");
  app.installTranslator(&translator);

  QDir pluginsDir = QDir(app.applicationDirPath());

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
    QPluginLoader *loader = new QPluginLoader(pluginsDir.absoluteFilePath(fileName));
    QObject *plugin = loader->instance();
    if (plugin)
    {
      qDebug() << "Found plugin " << fileName;;
      plugins << plugin;
      loaders << loader;
    } else
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

    res = app.exec();
  }

//   qDebug() << "\nUnloading Plugins: \n";
  foreach(QPluginLoader *plugin, loaders)
  {
    plugin->unload();
    delete plugin;
  }

  return res;
}
