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

#include "common/processing/pqFilter.h"

// ESPINA
#include "common/cache/CachedObjectBuilder.h"
#include "common/processing/pqData.h"
#include <vtkImageAlgorithm.h>

// Debug
#include <iostream>
#include <QDebug>

using namespace std;

pqFilter::pqFilter(vtkImageAlgorithm *algorithm, const QString& cacheId)
: m_algorithm(algorithm)
, m_id(cacheId)
{
}

pqFilter::~pqFilter()
{
  Q_ASSERT(false);
//    qDebug() << m_id << "has" << m_source->getNumberOfConsumers() << "consumers";
}

int pqFilter::getNumberOfData()
{
  return m_algorithm->GetNumberOfOutputPorts();
}

pqData pqFilter::data(int i)
{
  pqData filterData(this,i);
  return filterData;
}

QDebug operator<<(QDebug& out, const pqFilter::Argument& arg)
{
  out << "Argument("<< arg.name;
  switch (arg.type)
  {
    case pqFilter::Argument::INPUT:
      out << "Input";
      break;
    case pqFilter::Argument::INTVECT:
      out << "IntVector";
      break;
    case pqFilter::Argument::DOUBLEVECT:
      out << "DoubleVector";
      break;
    default:
      out << "UNKOWN";
  }
  out << arg.value << ")";
  return out;
}