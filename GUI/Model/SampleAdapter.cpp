/*
    
    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>

    This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
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

#include "GUI/Model/SampleAdapter.h"

#include <Core/Analysis/Sample.h>

using namespace EspINA;


//------------------------------------------------------------------------
SampleAdapter::SampleAdapter(SampleSPtr sample)
: NeuroItemAdapter(sample)
, m_sample(sample)
{

}

//------------------------------------------------------------------------
SampleAdapter::~SampleAdapter()
{

}

//------------------------------------------------------------------------
QVariant SampleAdapter::data(int role) const
{
  QVariant res;

  if (Qt::DisplayRole == role)
  {
    res = m_sample->name();
  }

  return res;
}

//------------------------------------------------------------------------
bool SampleAdapter::setData(const QVariant& value, int role)
{
  bool res = false;

  if (Qt::DisplayRole == role)
  {
    m_sample->setName(value.toString());
    res = true;
  }

  return res;
}

//------------------------------------------------------------------------
void SampleAdapter::setName(const QString& name)
{
  m_sample->setName(name);
}

//------------------------------------------------------------------------
QString SampleAdapter::name() const
{
  return m_sample->name();
}

//------------------------------------------------------------------------
void SampleAdapter::setPosition(const NmVector3& point)
{
  m_sample->setPosition(point);
}

//------------------------------------------------------------------------
NmVector3 SampleAdapter::position() const
{
  return m_sample->position();
}

//------------------------------------------------------------------------
void SampleAdapter::setBounds(const Bounds& bounds)
{
  m_sample->setBounds(bounds);
}

//------------------------------------------------------------------------
Bounds SampleAdapter::bounds() const
{
  return m_sample->bounds();
}

//------------------------------------------------------------------------
bool EspINA::operator==(SampleAdapterSPtr lhs, SampleSPtr rhs)
{
  return lhs->m_sample == rhs;
}

//------------------------------------------------------------------------
bool EspINA::operator==(SampleSPtr lhs, SampleAdapterSPtr rhs)
{
  return lhs == rhs->m_sample;
}

//------------------------------------------------------------------------
bool EspINA::operator!=(SampleAdapterSPtr lhs, SampleSPtr rhs)
{
  return !operator==(lhs, rhs);
}

//------------------------------------------------------------------------
bool EspINA::operator!=(SampleSPtr lhs, SampleAdapterSPtr rhs)
{
  return !operator==(lhs, rhs);
}

//------------------------------------------------------------------------
SampleAdapterPtr EspINA::samplePtr(ItemAdapterPtr item)
{
  return static_cast<SampleAdapterPtr>(item);
}
