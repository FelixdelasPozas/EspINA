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


#include "model/Sample.h"

#include <QDebug>

//------------------------------------------------------------------------
Sample::Sample(QString ID)
: m_ID(ID)
{
  qDebug() << "Created Sample:" << ID;
}

//------------------------------------------------------------------------
Sample::~Sample()
{
  qDebug() << "Deleted Sample:" << m_ID;
}

//------------------------------------------------------------------------
double* Sample::origin()
{
  return m_origin;
}


//------------------------------------------------------------------------
void Sample::setOrigin(double origin[3])
{
  memcpy(m_origin, origin, 3*sizeof(double));
}

//------------------------------------------------------------------------
double* Sample::size()
{
  return m_size;
}

//------------------------------------------------------------------------
void Sample::setSize(double size[3])
{
  memcpy(m_size, size, 3*sizeof(double));
}

//------------------------------------------------------------------------
void Sample::addChannel(Channel* channel)
{

}

//------------------------------------------------------------------------
void Sample::addSegmentation(Segmentation* seg)
{

}

//------------------------------------------------------------------------
QVariant Sample::data(int role) const
{
  if (Qt::DisplayRole == role)
    return m_ID;
  else
    return QVariant();
}