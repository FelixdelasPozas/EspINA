/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  Jorge PeÃ±a Pastor <email>

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


#include "MorphologicalEditionFilter.h"
#include "CODESettings.h"

#include <model/EspinaFactory.h>

#include <QDebug>
#include <QApplication>

typedef ModelItem::ArgumentId ArgumentId;
const ArgumentId MorphologicalEditionFilter::RADIUS = "Radius";

const unsigned int LABEL_VALUE = 255;


//-----------------------------------------------------------------------------
MorphologicalEditionFilter::MorphologicalEditionFilter(Filter::NamedInputs inputs,
                                                       ModelItem::Arguments args)
: Filter(inputs, args)
, m_params(m_args)
, m_input(NULL)
, m_needUpdate(false)
{
}


//-----------------------------------------------------------------------------
MorphologicalEditionFilter::~MorphologicalEditionFilter()
{
//   qDebug() << "Destroying" << TYPE;
}

//-----------------------------------------------------------------------------
bool MorphologicalEditionFilter::needUpdate() const
{
  return (!m_outputs.contains(0) || m_needUpdate);
}


//-----------------------------------------------------------------------------
bool MorphologicalEditionFilter::prefetchFilter()
{
  if (m_needUpdate)
    return false;

  return Filter::prefetchFilter();
}

//-----------------------------------------------------------------------------
QWidget* MorphologicalEditionFilter::createConfigurationWidget()
{
  return new CODESettings(this);
}
