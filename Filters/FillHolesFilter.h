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


#ifndef ESPINA_FILL_HOLES_FILTER_H
#define ESPINA_FILL_HOLES_FILTER_H

#include "Filters/EspinaFilters_Export.h"

#include <Core/Analysis/Filter.h>

namespace EspINA
{
  class EspinaFilters_EXPORT FillHolesFilter
  : public Filter
  {
  public:
    explicit FillHolesFilter(InputSList    inputs,
                             Filter::Type  type,
                             SchedulerSPtr scheduler);
    virtual ~FillHolesFilter();

    virtual void restoreState(const State& state);

    virtual State state() const;

  protected:
    virtual Snapshot saveFilterSnapshot() const;

    virtual bool needUpdate() const;

    virtual bool needUpdate(Output::Id id) const;

    virtual void execute()
    { execute(0); }

    virtual void execute(Output::Id id);

    virtual bool ignoreStorageContent() const
    { return false; }

    virtual bool invalidateEditedRegions();

  };

  using FillHolesFilterPtr  = FillHolesFilter *;
  using FillHolesFilterSPtr = std::shared_ptr<FillHolesFilter>;
} // namespace EspINA

#endif // ESPINA_FILL_HOLES_FILTER_H