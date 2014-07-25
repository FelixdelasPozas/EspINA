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


#ifndef ESPINA_DILATE_FILTER_H
#define ESPINA_DILATE_FILTER_H

#include "Filters/EspinaFilters_Export.h"

#include "MorphologicalEditionFilter.h"

namespace ESPINA
{
  class EspinaFilters_EXPORT DilateFilter
  : public MorphologicalEditionFilter
  {
  public:
    explicit DilateFilter(InputSList    inputs,
                          Filter::Type  type,
                          SchedulerSPtr scheduler);
    virtual ~DilateFilter();

  protected:
    virtual void execute()
    { execute(0); }

    virtual void execute(Output::Id id);
  };

} // namespace ESPINA


#endif // DILATEFILTER_H
