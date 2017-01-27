/*
 * Copyright (C) 2017, Rafael Juan Vicente Garcia <rafaelj.vicente@gmail.com>
 *
 * This file is part of ESPINA.
 *
 * ESPINA is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

// ESPINA
#include "SliceInterpolationFilter.h"
#include <Core/Analysis/Data/VolumetricData.hxx>

using namespace ESPINA;
using namespace ESPINA::Core::Utils;

//------------------------------------------------------------------------
SliceInterpolationFilter::SliceInterpolationFilter(InputSList inputs, const Filter::Type &type, SchedulerSPtr scheduler)
: Filter(inputs, type, scheduler)
{
}

//------------------------------------------------------------------------
bool ESPINA::SliceInterpolationFilter::needUpdate() const
{
  return m_outputs.isEmpty() || !validOutput(0);
}

//------------------------------------------------------------------------
void ESPINA::SliceInterpolationFilter::execute()
{
  if (m_inputs.size() != 1)
    {
      auto what = QObject::tr("Invalid number of inputs, number: %1").arg(m_inputs.size());
      auto details = QObject::tr("FillHolesFilter::execute(id) -> Invalid number of inputs, number: %1").arg(m_inputs.size());

      throw EspinaException(what, details);
    }

    auto input = m_inputs[0];
    auto inputVolume = readLockVolume(input->output());
    if (!inputVolume->isValid())
    {
      auto what = QObject::tr("Invalid input volume");
      auto details = QObject::tr("FillHolesFilter::execute(id) -> Invalid input volume");

      throw EspinaException(what, details);
    }

    reportProgress(0);

    //TODO

    if (!canExecute()) return;
    reportProgress(100);
}
