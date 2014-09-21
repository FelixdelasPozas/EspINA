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
			explicit FillHolesFilter(InputSList inputs, Filter::Type type, SchedulerSPtr scheduler);

			/** \brief FillHolesFilter class virtual destructor.
			 *
			 */
			virtual ~FillHolesFilter();

			/** \brief Implements Persistent::restoreState().
			 *
			 */
			virtual void restoreState(const State& state);

			/** \brief Implements Persistent::state().
			 *
			 */
			virtual State state() const;

		protected:
			/** \brief Implements Filter::saveFilterSnapshot().
			 *
			 */
			virtual Snapshot saveFilterSnapshot() const;

			/** \brief Implements Filter::needUpdate().
			 *
			 */
			virtual bool needUpdate() const;

			/** \brief Implements Filter::needUpdate(id).
			 *
			 */
			virtual bool needUpdate(Output::Id id) const;

			/** \brief Implements Filter::execute().
			 *
			 */
			virtual void execute()
			{	execute(0);	}

			/** \brief Implements Filter::execute(id).
			 *
			 */
			virtual void execute(Output::Id id);

			/** \brief
			 *
			 */
			virtual bool ignoreStorageContent() const
			{	return false;	}

			/** \brief Implements Filter::invalidateEditedRegions().
			 *
			 */
			virtual bool invalidateEditedRegions();

	};

	using FillHolesFilterPtr = FillHolesFilter *;
	using FillHolesFilterSPtr = std::shared_ptr<FillHolesFilter>;
} // namespace ESPINA

#endif // ESPINA_FILL_HOLES_FILTER_H
