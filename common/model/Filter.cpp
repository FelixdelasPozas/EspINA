/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  Jorge Peña Pastor <jpena@cesvima.upm.es>

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


#include "Filter.h"
#include <EspinaCore.h>

#include <QWidget>

typedef ModelItem::ArgumentId ArgumentId;

const ArgumentId Filter::ID     = ArgumentId("ID", true);
const ArgumentId Filter::INPUTS = ArgumentId("Inputs", true);
const ArgumentId Filter::EDIT   = ArgumentId("Edit", true);

unsigned int Filter::m_lastId = 0;

//----------------------------------------------------------------------------
void Filter::resetId()
{
  m_lastId = 0;
}

//----------------------------------------------------------------------------
QString Filter::generateId()
{
  return QString::number(m_lastId++);
}

//----------------------------------------------------------------------------
void Filter::update()
{
  QString filename = id() + ".mhd";
  //if (EspinaCore::instance()->temporalDir()
  prefetchFilter();
  run();
}


//----------------------------------------------------------------------------
QWidget* Filter::createConfigurationWidget()
{
  return new QWidget();
}
