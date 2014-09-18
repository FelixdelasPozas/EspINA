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

#ifndef ESPINA_CLOSING_FILTER_H
#define ESPINA_CLOSING_FILTER_H

#include "Filters/EspinaFilters_Export.h"

// ESPINA
#include "MorphologicalEditionFilter.h"

namespace ESPINA
{
	class EspinaFilters_EXPORT CloseFilter
	: public MorphologicalEditionFilter
	{
		public:
			/* \brief CloseFilter class constructor.
			 * \param[in] inputs, list of input smart pointers.
			 * \param[in] type, CloseFilter type.
			 * \param[in] scheduler, scheduler smart pointer.
			 *
			 */
			explicit CloseFilter(InputSList inputs, Filter::Type type, SchedulerSPtr scheduler);

			/* \brief CloseFilter class virtual destructor.
			 *
			 */
			virtual ~CloseFilter();

		protected:
			/* \brief Implements Filter::execute().
			 *
			 */
			virtual void execute()
			{	execute(0); }

			/* \brief Implements Filter::execute(id).
			 *
			 */
			virtual void execute(Output::Id id);
	};

} // namespace ESPINA

#endif // CLOSINGFILTER_H
