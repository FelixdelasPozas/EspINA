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

#include <model/EspinaFactory.h>

#include <QDebug>
#include <QApplication>

typedef ModelItem::ArgumentId ArgumentId;
const ArgumentId MorphologicalEditionFilter::RADIUS = ArgumentId("Radius", true);

const unsigned int LABEL_VALUE = 255;


//-----------------------------------------------------------------------------
MorphologicalEditionFilter::MorphologicalEditionFilter(Filter::NamedInputs inputs,
                                                       ModelItem::Arguments args)
: Filter(inputs, args)
, m_params(m_args)
, m_input(NULL)
, m_volume(NULL)
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
  return m_volume == NULL;
}


//-----------------------------------------------------------------------------
int MorphologicalEditionFilter::numberOutputs() const
{
  return m_volume?1:0;
}

//-----------------------------------------------------------------------------
EspinaVolume* MorphologicalEditionFilter::output(OutputNumber i) const
{
  if (m_volume && i == 0)
    return m_volume;

  Q_ASSERT(false);
  return NULL;
}

//-----------------------------------------------------------------------------
bool MorphologicalEditionFilter::prefetchFilter()
{
  QString tmpFile = id() + "_0.mhd";
  m_cachedFilter = tmpFileReader(tmpFile);

  if (m_cachedFilter.IsNotNull())
  {
    m_volume = m_cachedFilter->GetOutput();
    emit modified(this);
    return true;
  }

  return false;
}