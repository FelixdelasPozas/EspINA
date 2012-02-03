/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2011  Jorge Peña Pastor<jpena@cesvima.upm.es>

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
#include "pqData.h"

// Debug

// ESPINA
#include "cache/CachedObjectBuilder.h"

#include <pqPipelineSource.h>


using namespace std;


///-----------------------------------------------------------------------------
/// pqData
///----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
pqData::pqData(pqFilter* source, unsigned int portNumber)
: m_source(source)
, m_portNumber(portNumber)
{
  Q_ASSERT(portNumber <= source->getNumberOfData());
}

//-----------------------------------------------------------------------------
QString pqData::id() const
{
  return QString("%1:%2").arg(m_source->id()).arg(m_portNumber);
}

//-----------------------------------------------------------------------------
pqPipelineSource* pqData::pipelineSource()
{
  return m_source->pipelineSource();
}

//-----------------------------------------------------------------------------
pqOutputPort* pqData::outputPort() const
{
  return m_source->pipelineSource()->getOutputPort(m_portNumber);
}

//-----------------------------------------------------------------------------
// vtkProduct::vtkProduct(const QString &id)
// {
//   QStringList fields = id.split(":");
//   CachedObjectBuilder *cob = CachedObjectBuilder::instance();
//   m_creator =  cob->getFilter(fields[0]);
//   m_portNumber = fields[1].toInt();
//   assert(m_creator);
//   assert(m_portNumber >= 0);
// }



// ///-----------------------------------------------------------------------------
// /// Espina PRODUCT
// ///----------------------------------------------------------------------------
// //-----------------------------------------------------------------------------
// EspinaProduct::EspinaProduct(EspinaFilter *parent, vtkFilter* creator, int portNumber)
// : vtkProduct(creator, portNumber)
// //, IRenderable(creator->pipelineSource(),portNumber)
// , m_parent(parent)
// , m_taxonomy(NULL)
// , m_origin(NULL)
// , m_style(VISIBLE)
// {
//   //TODO: vetexId = 
//   type = PRODUCT;
//   m_rgba[0] = m_rgba[1] = m_rgba[2] = m_rgba[3] = 0.0;
// }
// 
// //-----------------------------------------------------------------------------
// void EspinaProduct::addArgument ( QString name, QString argValue )
// {
//   m_args.append(ESPINA_ARG(name,argValue));
// }
// 
// //-----------------------------------------------------------------------------
// QString EspinaProduct::getArgument(QString name) const
// {
//   return id();
// }
// 
// //-----------------------------------------------------------------------------
// QString EspinaProduct::getArguments() const
// {
//   QString args;
//   args.append(ESPINA_ARG("Id", id()));
//   args.append(ESPINA_ARG("Taxonomy", m_taxonomy?m_taxonomy->qualifiedName():""));
//   args.append(m_args);
//   return args;
// }
// 
// 
// //-----------------------------------------------------------------------------
// QVariant EspinaProduct::data(int role) const
// {
// 
// }
// 
// void EspinaProduct::color(double* rgba)
// {
//   QColor color = this->data(Qt::DecorationRole).value<QColor>();
//   rgba[0] = color.red()/255.0;
//   rgba[1] = color.green()/255.0;
//   rgba[2] = color.blue()/255.0;
//   rgba[3] = 1;
// }