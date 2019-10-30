/*

 Copyright (C) 2018 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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
#include <Core/Factory/CoreFactory.h>
#include <Core/Utils/EspinaException.h>
#include "Plugin.h"

// Qt
#include <QPluginLoader>

using namespace ESPINA;
using namespace ESPINA::Core::Utils;

//--------------------------------------------------------------------
QList<QPluginLoader*> ESPINA::Core::loadPlugins(const QDir& path, CoreFactory* factory)
{
  QList<QPluginLoader *> loaders;

  if(factory && path.isReadable() && path.exists())
  {
    for(auto fileName : path.entryList(QDir::Files))
    {
        auto loader = new QPluginLoader(path.absoluteFilePath(fileName));
        auto plugin = loader->instance();

        try
        {
          if(loadPlugin(plugin, factory))
          {
            loaders << loader;
          }
          else
          {
            delete loader;
          }
        }
        catch(const EspinaException &e)
        {
          qDebug() << "Error loading plugin" << loader->fileName();
          delete loader;
        }
     }
  }

  return loaders;
}

//--------------------------------------------------------------------
void ESPINA::Core::unloadPlugins(QList<QPluginLoader *> plugins)
{
  for(auto loader: plugins)
  {
    loader->unload();
    delete loader;
  }
}

//--------------------------------------------------------------------
bool ESPINA::Core::loadPlugin(QObject* instance, CoreFactory* factory)
{
  if (instance)
  {
    auto corePlugin = qobject_cast<Core::CorePlugin*>(instance);
    if(corePlugin)
    {
      qDebug() << "Loading Core Plugin:" << corePlugin->name();

      corePlugin->init(factory->scheduler());

      for (auto stackExtensionFactory : corePlugin->channelExtensionFactories())
      {
        qDebug() << corePlugin << "- Stack Extension Factory  ...... OK";
        factory->registerExtensionFactory(stackExtensionFactory);
      }

      for(auto segmentationExtensionFactory: corePlugin->segmentationExtensionFactories())
      {
        qDebug() << corePlugin << "- Segmentation Extension Factory  ...... OK";
        factory->registerExtensionFactory(segmentationExtensionFactory);
      }

      for(auto filterFactory: corePlugin->filterFactories())
      {
        qDebug() << corePlugin << "- Filter Factory  ...... OK";
        factory->registerFilterFactory(filterFactory);
      }

      for(auto reader: corePlugin->analysisReaders())
      {
        qDebug() << corePlugin << "- Analysis Reader ...... OK";
        factory->registerAnalysisReader(reader);
      }

      return true;
    }
  }

  return false;
}
