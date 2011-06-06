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
#include "filter.h"

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
#include <vtkSMRGBALookupTableProxy.h>
#include <vtkSMProperty.h>
#include <vtkSMProxyProperty.h>

using namespace std;


///-----------------------------------------------------------------------------
/// VTK PRODUCT
///----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
vtkProduct::vtkProduct(const QString &id)
{
  QStringList fields = id.split(":");
  CachedObjectBuilder *cob = CachedObjectBuilder::instance();
  m_creator =  cob->getFilter(fields[0]);
  m_portNumber = fields[1].toInt();
  assert(m_creator);
  assert(m_portNumber >= 0);
}

//-----------------------------------------------------------------------------
vtkProduct::vtkProduct(vtkFilter *creator, int portNumber)
: m_creator(creator)
, m_portNumber(portNumber)
{
  assert(m_creator);
  assert(m_portNumber >= 0);
}

//-----------------------------------------------------------------------------
QString vtkProduct::id() const
{
  return QString("%1:%2").arg(m_creator->id()).arg(m_portNumber);
}


//-----------------------------------------------------------------------------
pqOutputPort* vtkProduct::outputPort()
{
  return m_creator->pipelineSource()->getOutputPort(m_portNumber);
}


///-----------------------------------------------------------------------------
/// Espina PRODUCT
///----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
EspinaProduct::EspinaProduct(EspinaFilter *parent, vtkFilter* creator, int portNumber)
: vtkProduct(creator, portNumber)
//, IRenderable(creator->pipelineSource(),portNumber)
, m_parent(parent)
, m_taxonomy(NULL)
, m_origin(NULL)
, m_style(VISIBLE)
{
  //TODO: vetexId = 
  type = PRODUCT;
  m_rgba[0] = m_rgba[1] = m_rgba[2] = m_rgba[3] = 0.0;
}

//-----------------------------------------------------------------------------
QString EspinaProduct::getArgument(QString name) const
{
  return id();
}

//-----------------------------------------------------------------------------
QString EspinaProduct::getArguments() const
{
  QString args;
  args.append(ESPINA_ARG("Id", id()));
  args.append(ESPINA_ARG("Taxonomy", m_taxonomy?m_taxonomy->getName():""));
  return args;
}


//-----------------------------------------------------------------------------
QVariant EspinaProduct::data(int role) const
{

}

void EspinaProduct::color(double *rgba)
{
  QColor color = this->data(Qt::DecorationRole).value<QColor>();
  rgba[0] = color.red()/255.0;
  rgba[1] = color.green()/255.0;
  rgba[2] = color.blue()/255.0;
  rgba[3] = 1;
}

//-----------------------------------------------------------------------------
// Sample
//-----------------------------------------------------------------------------
Sample::~Sample()
{
  QStringList extList = m_extensions.keys();
  extList.sort();
  for (int ext = extList.size()-1; ext>=0; ext--)
    delete m_extensions[extList[ext]];
  
  QStringList repList = m_repMap.keys();
  repList.sort();
  for (int rep = repList.size()-1; rep>=0; rep--)
    delete m_repMap[repList[rep]];
  
  CachedObjectBuilder::instance()->removeFilter(this->creator());  
}


//-----------------------------------------------------------------------------
QString Sample::label() const
{
  return m_creator->id().split(":")[0];
}


//------------------------------------------------------------------------
QVariant Sample::data(int role) const
{
  switch (role)
  {
    case Qt::DisplayRole:
      return label();
    case Qt::DecorationRole:
      return QColor(Qt::blue);
    default:
      return QVariant();
  }
}


//------------------------------------------------------------------------
bool Sample::setData(const QVariant& value, int role)
{
  if (role == Qt::EditRole)
  {
  }
  return false;
}


//------------------------------------------------------------------------
void Sample::extent(int *out)
{
  //if (!m_extent)
  //{
    mutex.lock();
    m_creator->pipelineSource()->updatePipeline();;
    m_creator->pipelineSource()->getProxy()->UpdatePropertyInformation();
    vtkPVDataInformation *info = outputPort()->getDataInformation();
    m_extent = info->GetExtent();
    mutex.unlock();
  //}
  memcpy(out,m_extent,6*sizeof(int));
}

//------------------------------------------------------------------------
void Sample::bounds(double *out)
{
  //if (!m_bounds)
  //{
    mutex.lock();
    m_creator->pipelineSource()->updatePipeline();;
    m_creator->pipelineSource()->getProxy()->UpdatePropertyInformation();
    vtkPVDataInformation *info = outputPort()->getDataInformation();
    m_bounds = info->GetBounds();
    mutex.unlock();
  //}
  memcpy(out,m_bounds,6*sizeof(double));
}

//------------------------------------------------------------------------
void Sample::spacing(double* out)
{
  int e[6];
  double b[6];
  extent(e);
  bounds(b);
  out[0] = b[1] / e[1];
  out[1] = b[3] / e[3];
  out[2] = b[5] / e[5];
//   qDebug() << "Spacing";
//   qDebug() << e[0] << e[1] << e[2] << e[3] << e[4] << e[5];
//   qDebug() << b[0] << b[1] << b[2] << b[3] << b[4] << b[5];
//   qDebug() << out[0] << out[1] << out[2];
}

void Sample::addSegmentation(Segmentation* seg)
{
  m_segs.push_back(seg);
  foreach(ISampleRepresentation *rep, m_repMap)
  {
    rep->updateRepresentation();
  }
}

void Sample::removeSegmentation(Segmentation* seg)
{
  m_segs.removeOne(seg);
  foreach(ISampleRepresentation *rep, m_repMap)
  {
    rep->updateRepresentation();
  }
}

void Sample::addExtension(ISampleExtension* ext)
{
  ISampleExtension *extAdded = ext->clone();
  if (m_extensions.contains(ext->id()))
  {
    qDebug() << "Sample Extensions:" << ext->id() << "already registered";
    assert(false);
  }
  m_extensions[ext->id()] = extAdded;
}

ISampleExtension* Sample::extension(ExtensionId extId)
{
  assert(m_extensions.contains(extId));
  return m_extensions[extId];
}

void Sample::initialize()
{
  foreach(ISampleExtension *ext, m_extensions)
  {
    ext->initialize(this);
    ext->addInformation(m_infoMap);
    ext->addRepresentations(m_repMap);
  }
}




//-----------------------------------------------------------------------------
// Segmentation
//-----------------------------------------------------------------------------
Segmentation::Segmentation(EspinaFilter* parent, vtkFilter* creator, int portNumber)
: EspinaProduct(parent,creator, portNumber)
{
}

//------------------------------------------------------------------------
Segmentation::~Segmentation()
{
  foreach(ISegmentationExtension *ext, m_extensions)
    delete ext;
  
  foreach(ISegmentationRepresentation *rep, m_repMap)
    delete rep;
}

//------------------------------------------------------------------------
QVariant Segmentation::data(int role) const
{
  switch (role)
  {
    case Qt::DisplayRole:
    //case Qt::EditRole:
      return label();
    case Qt::DecorationRole:
      return m_taxonomy->getColor();
    case Qt::CheckStateRole:
      return visible()?Qt::Checked:Qt::Unchecked;
    default:
      return QVariant();
  }
}

//------------------------------------------------------------------------
bool Segmentation::setData(const QVariant& value, int role)
{
 if (role == Qt::EditRole)
    {
      //TODO: change segmentation name
    }
    if (role == Qt::CheckStateRole)
    {
      setVisible(value.toBool());
      return true;
    }
    return false;
}

//------------------------------------------------------------------------
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

//------------------------------------------------------------------------
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
