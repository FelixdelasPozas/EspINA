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


#include "common/model/Channel.h"
#include "common/model/Sample.h"
#include "common/model/Segmentation.h"
#include "common/extensions/SampleExtension.h"

#include <QDebug>

//------------------------------------------------------------------------
Sample::Sample(const QString id)
: m_ID(id)
{
  memset(m_position, 0, 3*sizeof(double));
  //TODO
  m_bounds[0] = m_bounds[2] = m_bounds[4] = 0;
  m_bounds[1] = 698;
  m_bounds[3] = 535;
  m_bounds[5] = 228;
  qDebug() << "Created Sample:" << id;
}

//------------------------------------------------------------------------
Sample::Sample(const QString id, const QString args)
: m_ID(id)
{
  memset(m_position, 0, 3*sizeof(double));
  //TODO
  m_bounds[0] = m_bounds[2] = m_bounds[4] = 0;
  m_bounds[1] = 698;
  m_bounds[3] = 535;
  m_bounds[5] = 228;
  qDebug() << "Created Sample:" << id;
}

//------------------------------------------------------------------------
Sample::~Sample()
{
  qDebug() << "Deleted Sample:" << m_ID;
}

//------------------------------------------------------------------------
void Sample::position(double pos[3])
{
  memcpy(pos, m_position, 3*sizeof(double));
}


//------------------------------------------------------------------------
void Sample::setPosition(double pos[3])
{
  memcpy(m_position, pos, 3*sizeof(double));
}

//------------------------------------------------------------------------
void Sample::bounds(double value[6])
{
  memcpy(value, m_bounds, 6*sizeof(double));
}

//------------------------------------------------------------------------
void Sample::setBounds(double value[6])
{
  memcpy(m_bounds, value, 6*sizeof(double));
}

//------------------------------------------------------------------------
void Sample::addChannel(Channel *channel)
{

}

//------------------------------------------------------------------------
void Sample::addSegmentation(Segmentation *seg)
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

//------------------------------------------------------------------------
void Sample::initialize(ModelItem::Arguments args)
{
  foreach(ModelItemExtension *ext, m_extensions)
  {
    SampleExtension *sampleExt = dynamic_cast<SampleExtension *>(ext);
    Q_ASSERT(sampleExt);
    sampleExt->initialize(this);
  }
}

//------------------------------------------------------------------------
void Sample::addExtension(SampleExtension *ext)
{
  ModelItem::addExtension(ext);
}

