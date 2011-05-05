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

#include "filter.h"

// ESPINA
#include "cache/cachedObjectBuilder.h"

// ParaQ
#include "pqPipelineSource.h"
#include "vtkSMProxy.h"

// Debug
#include <iostream>
#include <assert.h>
#include <QDebug>

#include "data/hash.h"
#include <pqOutputPort.h>
#include <vtkPVDataInformation.h>

using namespace std;

//-----------------------------------------------------------------------------
// FILTER
//-----------------------------------------------------------------------------
Filter::Filter(
  const QString& group
, const QString& name
, const EspinaParamList& args
, const TranslatorTable &table
  )
  : m_args(args)
  , m_translator(table)
  //, m_filtertrace(name)
{
  this->name = QString(group).append("::").append(name);
  this->type = 1;
  
  ProcessingTrace* trace = ProcessingTrace::instance();
  trace->addNode(this);
  
  CachedObjectBuilder *cob = CachedObjectBuilder::instance();
  
  VtkParamList vtkArgs;
  vtkArgs = m_translator.translate(args);
  
  m_proxy = cob->createFilter(group,name,vtkArgs);
  
  m_proxy->getProxy()->UpdateVTKObjects();
  
  for (int portNumber = 0; portNumber < m_proxy->getOutputPorts().size(); portNumber++)
  {
    //TODO:WARNING:Que hacer con los parametros que se pasan al producto
    Product *filterOutput = new Product(m_proxy,portNumber, "Product" , this->id());
    //filterOutput->m_parentHash = this->id(); //TODO modify the way it takes the parent hash, Maybe in the constructer (above line)
    trace->addNode(filterOutput);
    trace->connect(this,filterOutput,"segmentation");
    m_products.push_back(filterOutput);
  }
  qDebug() << "Filter: Created Filter with id " << this->id();
}

//-----------------------------------------------------------------------------
/*
vector< ITraceNode* > Filter::inputs()
{
 return m_filtertrace.inputs(this);
}

//-----------------------------------------------------------------------------
vector< ITraceNode* > Filter::outputs()
{
 return m_filtertrace.outputs(this);
}
*/
//-----------------------------------------------------------------------------
void Filter::print(int indent) const
{
  cout << name.toStdString().c_str() << endl;
}

//-----------------------------------------------------------------------------
EspinaParamList Filter::getArguments()
{
  //EspinaParamList nullParamList;
  return m_args;
}

//-----------------------------------------------------------------------------
QString Filter::id()
{
  QStringList namesToHash;
  namesToHash.push_back(name);
  namesToHash.append( reduceArgs(m_args) );
  return generateSha1(namesToHash);
}


vector<Product *> Filter::products()
{
  return m_products;
}
