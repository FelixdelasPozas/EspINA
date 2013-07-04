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

#include <GUI/ViewManager.h>
#include <GUI/Extensions/Visualization/VisualizationState.h>
#include <EspinaMainWindow.h>
#include <Core/Model/EspinaFactory.h>
#include <Core/Extensions/EdgeDistances/AdaptiveEdges.h>
#include <Core/Extensions/EdgeDistances/EdgeDistance.h>
#include <Core/Extensions/Morphological/MorphologicalInformation.h>
#include <Core/Extensions/Tags/TagExtension.h>
#include <Core/Extensions/Notes/SegmentationNotes.h>

int main(int argc, char **argv)
{
  QApplication app(argc, argv);
  app.setWindowIcon(QIcon(":/espina/espina.svg"));

  QTranslator translator;
  translator.load("espina_es");
  app.installTranslator(&translator);

  EspINA::EspinaFactory factory;
  EspINA::EspinaModel   model(&factory);
  EspINA::ViewManager   viewManager;

  QDir pluginsDir = QDir(app.applicationDirPath());

  #if defined(Q_OS_WIN)
  if (pluginsDir.dirName().toLower() == "debug" || pluginsDir.dirName().toLower() == "release")
    pluginsDir.cdUp();
  #elif defined(Q_OS_MAC)
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
  foreach (QString fileName, pluginsDir.entryList(QDir::Files))
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
      // qDebug() << fileName << "not loaded -> Error:" << loader->errorString();
      delete loader;
    }
  }

  int res = 0;
  {
    EspINA::AdaptiveEdges            adaptiveEdgesExtension;
    EspINA::EdgeDistance             edgeDistanceExtension;
    EspINA::MorphologicalInformation morphologicalExtension;
    EspINA::SegmentationNotes        notesExtension;
    EspINA::SegmentationTags         tagsExtension;
    EspINA::VisualizationState       visualizationExtension;

    factory.registerChannelExtension     (&adaptiveEdgesExtension);
    factory.registerSegmentationExtension(&edgeDistanceExtension);
    factory.registerSegmentationExtension(&morphologicalExtension);
    factory.registerSegmentationExtension(&notesExtension);
    factory.registerSegmentationExtension(&tagsExtension);
    factory.registerSegmentationExtension(&visualizationExtension);

    EspINA::EspinaMainWindow espina(&model, &viewManager, plugins);
    espina.show();

    res = app.exec();

    factory.unregisterSegmentationExtension(&visualizationExtension);
    factory.unregisterSegmentationExtension(&tagsExtension);
    factory.unregisterSegmentationExtension(&notesExtension);
    factory.unregisterSegmentationExtension(&morphologicalExtension);
    factory.unregisterSegmentationExtension(&edgeDistanceExtension);
    factory.unregisterChannelExtension     (&adaptiveEdgesExtension);
  }

//   qDebug() << "\nUnloading Plugins: \n";
  foreach(QPluginLoader *plugin, loaders)
  {
    plugin->unload();
    delete plugin;
  }

  return res;
}
