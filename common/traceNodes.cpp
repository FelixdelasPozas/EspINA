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
#include <pqOutputPort.h>
#include <vtkPVDataInformation.h>
using namespace std;

//-----------------------------------------------------------------------------
// PRODUCT
//-----------------------------------------------------------------------------
Product::Product(pqPipelineSource* source, int portNumber, const QString &traceName, const QString &parentHash)
: IRenderable(source, portNumber), 
  m_parentHash(parentHash)
{
  this->name = traceName;
  this->type = 0;
  m_parentHash = parentHash;
}
/*
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
*/
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
//! Returns the id of the Product composed with the parent id and its Product name
QString Product::id()
{
  QStringList v;
  v.push_back( name );
  QString id = QString( m_parentHash );
  return id.append(generateSha1( v ));
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

void Sample::extent(int *out)
{
  //if (!m_extent)
  //{
    mutex.lock();
    sourceData()->updatePipeline();
    sourceData()->getProxy()->UpdatePropertyInformation();
    vtkPVDataInformation *info = outputPort()->getDataInformation();
    m_extent = info->GetExtent();
    mutex.unlock();
  //}
  memcpy(out,m_extent,6*sizeof(int));
}

void Sample::bounds(double *out)
{
  //if (!m_bounds)
  //{
    mutex.lock();
    sourceData()->updatePipeline();
    sourceData()->getProxy()->UpdatePropertyInformation();
    vtkPVDataInformation *info = outputPort()->getDataInformation();
    m_bounds = info->GetBounds();
    mutex.unlock();
  //}
  memcpy(out,m_bounds,6*sizeof(double));
}

void Sample::spacing(double* out)
{
  //TODO: Sorry, but no time to make it better
  double spacing[3];
  int e[6];
  double b[6];
  extent(e);
  bounds(b);
  out[0] = b[1] / e[1];
  out[1] = b[3] / e[3];
  out[2] = b[5] / e[5];
  qDebug() << "Spacing";
  qDebug() << e[0] << e[1] << e[2] << e[3] << e[4] << e[5];
  qDebug() << b[0] << b[1] << b[2] << b[3] << b[4] << b[5];
  qDebug() << out[0] << out[1] << out[2];
}



//-----------------------------------------------------------------------------
// Segmentation
//-----------------------------------------------------------------------------
Segmentation::Segmentation(pqPipelineSource* source, int portNumber, const QString& parentHash)
: Product(source, portNumber, "Segmentation", parentHash)
{ }

QVariant Segmentation::data(int role) const
{
  switch (role)
  {
    case Qt::DisplayRole:
    case Qt::EditRole:
      return name;
    case Qt::DecorationRole:
      return m_taxonomy->getColor();
    case Qt::CheckStateRole:
      return visible()?Qt::Checked:Qt::Unchecked;
    default:
      return QVariant();
  }
}

void Segmentation::addExtension(ISegmentationExtension* ext)
{
  ISegmentationExtension *extAdded = ext->clone();
  if (m_extensions.contains(ext->id()))
  {
    qDebug() << "Extension already registered";
    assert(false);
  }
  m_extensions[ext->id()] = extAdded;
}

ISegmentationExtension *Segmentation::extension(ExtensionId extId)
{
  assert(m_extensions.contains(extId));
  return m_extensions[extId];
}

//! TODO: Review where extensions should be initialized: at creation
//! or when adding them to EspINA
void Segmentation::initialize()
{
  foreach(ISegmentationExtension *ext, m_extensions)
  {
    ext->initialize(this);
    ext->addInformation(m_infoMap);
    ext->addRepresentations(m_repMap);
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
    Product *filterOutput = new Product(m_proxy,portNumber, this->id());
    //filterOutput->m_parentHash = this->id(); //TODO modify the way it takes the parent hash, Maybe in the constructer (above line)
    trace->addNode(filterOutput);
    trace->connect(this,filterOutput,"segmentation");
    m_products.push_back(filterOutput);
  }
  qDebug() << "Filter ID " << this->id();
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

// ProcessingTrace* Filter::trace()
// {
//   return &m_filtertrace;
// }

