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

#include "cache/cachedObjectBuilder.h"

#include "pqPipelineSource.h"

#include <iostream>


pqOutputPort *Product::outPut()
{
  return source->getOutputPort(0);
}

void Product::print(int indent) const
{
}

ParamList Product::getArguments()
{
  ParamList p;
  return p;
}


std::string Product::id()
{

}




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
  //CachedObjectBuilder *cob = CachedObjectBuilder::instance();
  
  //m_proxy = cob->createFilter(group,name,args);
}


void Filter::print(int indent) const
{
  std::cout << name << std::endl;
}

ParamList Filter::getArguments()
{
  ParamList p;
  return p;
}


std::string Filter::id()
{
  return name;
}


