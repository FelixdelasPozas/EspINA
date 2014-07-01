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


#ifndef ESPINA_COMPUTE_OPTIMAL_MARGINS_H
#define ESPINA_COMPUTE_OPTIMAL_MARGINS_H

#include <Core/MultiTasking/Task.h>
#include <Core/Utils/Bounds.h>
#include <GUI/Model/SegmentationAdapter.h>
#include <Extensions/ExtensionUtils.h>
#include <Extensions/EdgeDistances/EdgeDistance.h>
#include <vtkMath.h>
#include <QDebug>
#include <QElapsedTimer>

namespace EspINA
{

  template <typename C, typename S>
  class ComputeOptimalMargins
  : public Task
  {
  public:
    explicit ComputeOptimalMargins(C channel,
                                   S segmentations,
                                   SchedulerSPtr scheduler = SchedulerSPtr());
    virtual ~ComputeOptimalMargins();

    C channel() const
    { return m_channel;}

    void inclusion(Nm value[3]) const
    { memcpy(value, m_inclusion, 3*sizeof(Nm)); }

    void exclusion(Nm value[3]) const
    { memcpy(value, m_exclusion, 3*sizeof(Nm)); }

  protected:
    virtual void run();

  private:
    C m_channel;
    S m_segmentations;

    Nm m_inclusion[3];
    Nm m_exclusion[3];
  };
}// namespace EspINA

#include "ComputeOptimalMargins.cpp"

#endif // ESPINA_COMPUTE_OPTIMAL_MARGINS_H
