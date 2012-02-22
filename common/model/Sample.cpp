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


#include "common/model/Sample.h"

#include <QDebug>

//------------------------------------------------------------------------
Sample::Sample(const QString id)
: m_ID(id)
{
  bzero(m_position,3*sizeof(int));
  qDebug() << "Created Sample:" << id;
}

//------------------------------------------------------------------------
Sample::Sample(const QString id, const QString args)
: m_ID(id)
{
  bzero(m_position,3*sizeof(int));
  qDebug() << "Created Sample:" << id;
}

//------------------------------------------------------------------------
Sample::~Sample()
{
  qDebug() << "Deleted Sample:" << m_ID;
}

//------------------------------------------------------------------------
void Sample::position(int pos[3])
{
  memcpy(pos, m_position, 3*sizeof(int));
}


//------------------------------------------------------------------------
void Sample::setPosition(int pos[3])
{
  memcpy(m_position, pos, 3*sizeof(int));
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
void Sample::addChannel(ChannelPtr channel)
{

}

//------------------------------------------------------------------------
void Sample::addSegmentation(SegmentationPtr seg)
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