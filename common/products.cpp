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

#include "products.h"

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
Product::Product(pqPipelineSource* source, int portNumber, const QString& traceName, const EspinaId& parentHash)
: IRenderable(source, portNumber), 
  m_parentHash(parentHash), //TODO: Deprecate?
  m_taxonomy(NULL)
{
  this->name = traceName;
  this->type = 0;
  m_hash = QString("%1:%2").arg(parentHash).arg(portNumber);
  //QStringList v;
  //v.push_back( QString(portNumber) );
  //m_hash.append(generateSha1( v ));
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
  if( m_taxonomy )
    nullParamList.push_back(EspinaParam("Taxonomy", m_taxonomy->getName()));
  return nullParamList;
}


//-----------------------------------------------------------------------------
//! Returns the id of the Product composed with the parent id and its Product name
EspinaId Product::id()
{
//   QStringList v;
//   v.push_back( name );
//   QString id = m_hash;
//   id.append(generateSha1( v ));
// 
//   return id;
  return m_hash;
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
