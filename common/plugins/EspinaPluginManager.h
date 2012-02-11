/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2011  Jorge Peña <jorge.pena.pastor@gmail.com>

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


#ifndef ESPINAPLUGINMANAGER_H
#define ESPINAPLUGINMANAGER_H


#include <QString>
#include <QMap>

class IPreferencePanel;
class pqPipelineSource;
class IFilterFactory;
class IFileReader;
class EspinaFilter;

class EspinaPluginManager
{
public:
  EspinaPluginManager(){}
  ~EspinaPluginManager(){}

  //! Returns a SelectionManager singleton
  static EspinaPluginManager *instance()
  {
    if (!m_singleton)
      m_singleton = new EspinaPluginManager();
    return m_singleton;
  }
  
  void registerFilter(const QString &filter, IFilterFactory *factory);
//   EspinaFilter *createFilter(const QString& filter, ITraceNode::Arguments& args);
  
  void registerReader(const QString &extension, IFileReader *reader);
  void readFile(pqPipelineSource *proxy, const QString &filePath);
  
  void registerPreferencePanel(IPreferencePanel *panel);
  QList<IPreferencePanel *> preferencePanels() {return m_panels;}
    
private:
  EspinaPluginManager(const EspinaPluginManager &);// Disable copy constructor
  void operator=(const EspinaPluginManager &); // Disable copy assigment constructor
  
  static EspinaPluginManager *m_singleton;
  
  QMap<QString, IFilterFactory *> m_filters;
  QMap<QString, IFileReader *> m_readers;
  QList<IPreferencePanel *> m_panels;
};

#endif // ESPINAPLUGINMANAGER_H
