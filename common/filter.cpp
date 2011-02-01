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

// Debug
#include <iostream>
#include <assert.h>

//-----------------------------------------------------------------------------
// PRODUCT
//-----------------------------------------------------------------------------
std::vector< ITraceNode* > Product::inputs()
{
  return m_trace->inputs(this);
}

//-----------------------------------------------------------------------------
std::vector< ITraceNode* > Product::outputs()
{
  return m_trace->outputs(this);
}

//-----------------------------------------------------------------------------
void Product::print(int indent) const
{
}

//-----------------------------------------------------------------------------
ParamList Product::getArguments()
{
  ParamList p;
  return p;
}

//-----------------------------------------------------------------------------
std::string Product::id()
{
  std::string pId = name;// Use translator to generate own id.
  assert(this->inputs().size() == 1);// Products are only created by a filter
  Filter * parent = dynamic_cast<Filter *>(this->inputs().front());
  return parent->id() + pId;
}

//-----------------------------------------------------------------------------
//! Returns the vtk outputport
pqOutputPort *Product::outPut()
{
  return m_outputPort;
}

void Product::setOutputPort(pqOutputPort* port)
{
  m_outputPort = port;
}


//-----------------------------------------------------------------------------
// FILTER
//-----------------------------------------------------------------------------
Filter::Filter(
  const std::string& group
, const std::string& name
, const ParamList& args
, const TranslatorTable &table
  )
  : m_args(args)
  , m_translator(table)
{
  this->name = group + "::" + name;
  
  CachedObjectBuilder *cob = CachedObjectBuilder::instance();
  
  ParamList vtkArgs;
  vtkArgs = m_translator.translate(args);
  
  m_proxy = cob->createFilter(group,name,vtkArgs);
  
  qDebug() << m_proxy->getOutputPorts().size();
}

//-----------------------------------------------------------------------------
std::vector< ITraceNode* > Filter::inputs()
{
 return m_trace->inputs(this);
}

//-----------------------------------------------------------------------------
std::vector< ITraceNode* > Filter::outputs()
{
 return m_trace->outputs(this);
}

//-----------------------------------------------------------------------------
void Filter::print(int indent) const
{
  std::cout << name << std::endl;
}

//-----------------------------------------------------------------------------
ParamList Filter::getArguments()
{
  ParamList p;
  return p;
}


//-----------------------------------------------------------------------------
std::string Filter::id()
{
  //TODO: ParamList to Id
  return name;
}


std::vector<Product *> Filter::products()
{
  return m_products;
}

