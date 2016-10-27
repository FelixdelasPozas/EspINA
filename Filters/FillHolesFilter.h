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

// ESPINA
#include <Core/Analysis/Filter.h>

namespace ESPINA
{
	class EspinaFilters_EXPORT FillHolesFilter
	: public Filter
	{
		public:
			/** \brief FillHolesFilter class constructor.
			 * \param[in] inputs, list of input smart pointers.
			 * \param[in] type, FillHolesFilter type.
			 * \param[in] scheduler, scheduler smart pointer.
			 *
			 */
			explicit FillHolesFilter(InputSList inputs, const Filter::Type &type, SchedulerSPtr scheduler);

			/** \brief FillHolesFilter class virtual destructor.
			 *
			 */
			virtual ~FillHolesFilter()
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

	using FillHolesFilterPtr = FillHolesFilter *;
	using FillHolesFilterSPtr = std::shared_ptr<FillHolesFilter>;
} // namespace ESPINA

#endif // ESPINA_FILL_HOLES_FILTER_H
