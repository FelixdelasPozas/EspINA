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

#include "traceNodes.h"

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
using namespace std;

//-----------------------------------------------------------------------------
// PRODUCT
//-----------------------------------------------------------------------------
Product::Product(pqPipelineSource* source, int portNumber)
: IRenderable(source, portNumber)
{
  this->name = "Product";
}

vector< ITraceNode* > Product::inputs()
{
  vector<ITraceNode *> nullVector;
  return nullVector;
}

//-----------------------------------------------------------------------------
vector< ITraceNode* > Product::outputs()
{
  vector<ITraceNode *> nullVector;
  return nullVector;
}

void Product::print(int indent) const
{
  cout << name.toStdString().c_str() << endl;
}

//-----------------------------------------------------------------------------
EspinaParamList Product::getArguments()
{
  EspinaParamList nullParamList;
  return nullParamList;
}


//-----------------------------------------------------------------------------
QString Product::id()
{
  vector<QString> v;
  v.push_back( name );
  QString id;
  id.append( m_parentHash );
  return id.append(generateSha1( v ));
  
//   string pId = name;// Use translator to generate own id.
//   return name; //DEBUG
//   assert(this->inputs().size() == 1);// Products are only created by a filter
//   Filter * parent = dynamic_cast<Filter *>(this->inputs().front());
//   return parent->id() + pId;
}

//-----------------------------------------------------------------------------
pqOutputPort* Product::outputPort()
{
  return IRenderable::outputPort();
}


//-----------------------------------------------------------------------------
pqPipelineSource* Product::sourceData()
{
  return IRenderable::sourceData();
}


//-----------------------------------------------------------------------------
int Product::portNumber()
{

  return IRenderable::portNumber();
}

void Product::color(double *rgba)
{
  QColor color = this->data(Qt::DecorationRole).value<QColor>();
  rgba[0] = color.red()/255.0;
  rgba[1] = color.green()/255.0;
  rgba[2] = color.blue()/255.0;
  rgba[3] = 1;
}

QVariant Product::data(int role) const
{
  switch (role)
  {
    case Qt::DisplayRole:
	return "Generic Product";
    case Qt::DecorationRole:
	return QColor(Qt::darkMagenta);
    default:
      return QVariant();
  }
}

//-----------------------------------------------------------------------------
// Sample
//-----------------------------------------------------------------------------
QVariant Sample::data(int role) const
{
  switch (role)
  {
    case Qt::DisplayRole:
      return name;
    case Qt::DecorationRole:
      return QColor(Qt::blue);
    default:
      return QVariant();
  }
}

//-----------------------------------------------------------------------------
// Segmentation
//-----------------------------------------------------------------------------
QVariant Segmentation::data(int role) const
{
  switch (role)
  {
    case Qt::DisplayRole:
      return "Segmentation";
    case Qt::DecorationRole:
      return m_taxonomy->getColor();
    default:
      return QVariant();
  }
}

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
  , m_filtertrace(name)
{
  this->name = QString(group).append("::").append(name);
  
  m_filtertrace.addNode(this);
  
  CachedObjectBuilder *cob = CachedObjectBuilder::instance();
  
  VtkParamList vtkArgs;
  vtkArgs = m_translator.translate(args);
  
  m_proxy = cob->createFilter(group,name,vtkArgs);
  
  m_proxy->getProxy()->UpdateVTKObjects();
  
  for (int portNumber = 0; portNumber < m_proxy->getOutputPorts().size(); portNumber++)
  {
    Product *filterOutput = new Product(m_proxy,portNumber);
    filterOutput->m_parentHash = this->id(); //TODO modify the way it takes the parent hash, Maybe in the constructer (above line)
    m_filtertrace.addNode(filterOutput);
    m_filtertrace.connect(this,filterOutput,"segmentation");
    m_products.push_back(filterOutput);
  }
  qDebug() << "Filter ID " << this->id();
}

//-----------------------------------------------------------------------------
vector< ITraceNode* > Filter::inputs()
{
 return m_filtertrace.inputs(this);
}

//-----------------------------------------------------------------------------
vector< ITraceNode* > Filter::outputs()
{
 return m_filtertrace.outputs(this);
}

//-----------------------------------------------------------------------------
void Filter::print(int indent) const
{
  cout << name.toStdString().c_str() << endl;
}

//-----------------------------------------------------------------------------
EspinaParamList Filter::getArguments()
{
  EspinaParamList nullParamList;
  return nullParamList;
}

//-----------------------------------------------------------------------------
QString Filter::id()
{
  //TODO: ParamList to Id
  std::vector<QString> namesToHash, argsToHash;
  namesToHash.push_back(name);
  argsToHash = reduceArgs( m_args );
  namesToHash.insert( namesToHash.end(), argsToHash.begin(), argsToHash.end());
  return generateSha1(namesToHash);
}


vector<Product *> Filter::products()
{
  return m_products;
}

ProcessingTrace* Filter::trace()
{
  return &m_filtertrace;
}

