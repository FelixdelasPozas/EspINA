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

// Debug
#include "products.h"

// ESPINA
#include "cache/cachedObjectBuilder.h"
#include "filter.h"
#include "espina_debug.h"
#include "data/hash.h"

// ParaQ
#include <pqApplicationCore.h>
#include <pqObjectBuilder.h>
#include <pqOutputPort.h>
#include <pqPipelineSource.h>

#include <vtkSMProxy.h>
#include <vtkPVDataInformation.h>
#include <vtkSMRGBALookupTableProxy.h>
#include <vtkSMProperty.h>
#include <vtkSMProxyProperty.h>
#include <vtkSMPropertyHelper.h>
#include "spatialExtension.h"


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
pqOutputPort* vtkProduct::outputPort() const
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
void EspinaProduct::addArgument ( QString name, QString argValue )
{
  m_args.append(ESPINA_ARG(name,argValue));
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
  args.append(m_args);
  return args;
}


//-----------------------------------------------------------------------------
QVariant EspinaProduct::data(int role) const
{

}

void EspinaProduct::color(double* rgba)
{
  QColor color = this->data(Qt::DecorationRole).value<QColor>();
  rgba[0] = color.red()/255.0;
  rgba[1] = color.green()/255.0;
  rgba[2] = color.blue()/255.0;
  rgba[3] = 1;
}