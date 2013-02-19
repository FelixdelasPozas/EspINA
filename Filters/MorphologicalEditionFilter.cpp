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

#include <Core/Model/EspinaFactory.h>

#include <QDebug>

using namespace EspINA;

typedef ModelItem::ArgumentId ArgumentId;
const ArgumentId MorphologicalEditionFilter::RADIUS = "Radius";

const unsigned int LABEL_VALUE = 255;


//-----------------------------------------------------------------------------
MorphologicalEditionFilter::MorphologicalEditionFilter(NamedInputs inputs,
                                                       Arguments   args,
                                                       FilterType  type)
: SegmentationFilter(inputs, args, type)
, m_params(m_args)
, m_input(NULL)
, m_paramModified(false)
{
}

//-----------------------------------------------------------------------------
MorphologicalEditionFilter::~MorphologicalEditionFilter()
{
}

//-----------------------------------------------------------------------------
bool MorphologicalEditionFilter::needUpdate() const
{
  bool update = m_paramModified || Filter::needUpdate();

  if (!update)
  {
    Q_ASSERT(m_namedInputs.size()  == 1);
    Q_ASSERT(m_outputs.size() == 1);
    Q_ASSERT(m_outputs[0].volume.get());
    Q_ASSERT(m_outputs[0].volume->toITK().IsNotNull());

    if (!m_inputs.isEmpty())
    {
      Q_ASSERT(m_inputs.size() == 1);
      update = m_outputs[0].volume->toITK()->GetTimeStamp() < m_inputs[0]->toITK()->GetTimeStamp();
    }
  }

  return update;
}


//-----------------------------------------------------------------------------
bool MorphologicalEditionFilter::fetchSnapshot()
{
  if (m_paramModified)
    return false;

  return Filter::fetchSnapshot();
}