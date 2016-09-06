/*
* Copyright (C) 2016, Rafael Juan Vicente Garcia <rafaelj.vicente@gmail.com>
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

#ifndef FILTERS_FILLHOLES2DFILTER_H_
#define FILTERS_FILLHOLES2DFILTER_H_

#include "Filters/EspinaFilters_Export.h"

// ESPINA
#include <Core/Analysis/Filter.h>

namespace ESPINA
{
	class EspinaFilters_EXPORT FillHoles2DFilter
	: public Filter
	{
    public:
      /** \brief FillHoles2DFilter class constructor.
       * \param[in] inputs list of input smart pointers.
       * \param[in] type FillHolesFilter type.
       * \param[in] scheduler scheduler smart pointer.
       *
       */
      FillHoles2DFilter(InputSList inputs, const Filter::Type &type, SchedulerSPtr scheduler);

      /** \brief FillHoles2DFilter class virtual destructor.
       *
       */
      virtual ~FillHoles2DFilter()
      {};

      virtual void restoreState(const State& state)
      {};

      virtual State state() const
      { return State(); };

    protected:
      virtual Snapshot saveFilterSnapshot() const
      { return Snapshot(); };

      virtual bool needUpdate() const;

      virtual void execute();

      virtual bool ignoreStorageContent() const
      {	return false;	}
	};

	using FillHoles2DFilterPtr = FillHoles2DFilter *;
	using FillHoles2DFilterSPtr = std::shared_ptr<FillHoles2DFilter>;

} // namespace ESPINA

#endif /* FILTERS_FILLHOLES2DFILTER_H_ */
