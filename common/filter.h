/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) <year>  <name of author>

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

#ifndef FILTER_H
#define FILTER_H

#include "processingTrace.h"
#include "products.h"
#include "interfaces.h"
#include "translatorTable.h"
#include "data/modelItem.h"
#include "EspinaPlugin.h"

#include <QString>
#include <data/taxonomy.h>
#include <Utilities/vxl/vcl/iso/vcl_iostream.h>

#include <QMutex>

// Forward declarations
class pqPipelineSource;
class Sample;


class Filter : public ITraceNode, public ISingleton
{
  
public:
  Filter(
    //! Paraview filter's group name
    const QString &group
    //! Paraview filter's name
    , const QString &name
    //! Espina Args list
    , const EspinaParamList &args
    //! Filter Translation Table
  , const TranslatorTable &table  
  );
  
  //! Implements ITraceNode interface
  virtual void print(int indent = 0) const;
  virtual EspinaParamList getArguments();
  
  //! Implements ISingleton
  virtual EspinaId id();
  
  QString group(){return m_group;}
  VtkParamList vtkArgs(){return m_vtkArgs;}
  
  std::vector<Product *> products();
  //ProcessingTrace *trace();
  
private:
  //void createFilter();
  QString m_group;
  
  EspinaParamList m_args;
  VtkParamList m_vtkArgs;
  EspinaProxy *m_proxy;
  const TranslatorTable &m_translator;
  //ProcessingTrace m_filtertrace;
  std::vector<Product *> m_products;
};

#endif // FILTER_H
