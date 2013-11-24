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

#include <GUI/ModelFactory.h>
#include <GUI/Representations/SliceRepresentation.h>

#include <QDebug>

using namespace EspINA;

const unsigned int LABEL_VALUE = 255;

//-----------------------------------------------------------------------------
template<class T>
MorphologicalEditionFilter<T>::MorphologicalEditionFilter(OutputSList   inputs,
                                                       Type          type,
                                                       SchedulerSPtr scheduler)
: BasicSegmentationFilter(inputs, type, scheduler)
, m_ignoreCurrentOutputs(false)
, m_isOutputEmpty(true)
, m_radius(0)
{
}

//-----------------------------------------------------------------------------
template<class T>
MorphologicalEditionFilter<T>::~MorphologicalEditionFilter()
{

}

//-----------------------------------------------------------------------------
template<class T>
bool MorphologicalEditionFilter<T>::needUpdate(Output::Id oId) const
{
  bool update = Filter::needUpdate(oId);

  if (!update)
  {
    Q_ASSERT(m_outputs.size() == 1);

    VolumetricDataSPtr<T> outputVolume = segmentationVolume(m_outputs[0]);
    Q_ASSERT(outputVolume.get());
    Q_ASSERT(outputVolume->toITK().IsNotNull());
    if (!m_inputs.isEmpty())
    {
      Q_ASSERT(m_inputs.size() == 1);
      SegmentationVolumeSPtr inputVolume = segmentationVolume(m_inputs[0]);
      update = outputVolume->timeStamp() < inputVolume->timeStamp();
    }
  }

  return update;
}
