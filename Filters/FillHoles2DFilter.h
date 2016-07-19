/*
 * FillHoles2DFilter.h
 *
 *  Created on: 12 de jul. de 2016
 *      Author: heavy
 */

#ifndef FILTERS_FILLHOLES2DFILTER_H_
#define FILTERS_FILLHOLES2DFILTER_H_

#include "Filters/EspinaFilters_Export.h"

// ESPINA
#include <Core/Analysis/Filter.h>

namespace ESPINA
{
	class EspinaFilters_EXPORT FillHoles2DFilter
	: public Filter {
	public:
		/** \brief FillHoles2DFilter class constructor.
		 * \param[in] inputs, list of input smart pointers.
		 * \param[in] type, FillHolesFilter type.
		 * \param[in] scheduler, scheduler smart pointer.
		 *
		 */
		FillHoles2DFilter(InputSList inputs, Filter::Type type, SchedulerSPtr scheduler);

		/** \brief FillHoles2DFilter class virtual destructor.
		 */
		virtual ~FillHoles2DFilter();

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
		virtual bool areEditedRegionsInvalidated();
	};

	using FillHoles2DFilterPtr = FillHoles2DFilter *;
	using FillHoles2DFilterSPtr = std::shared_ptr<FillHoles2DFilter>;

} // namespace ESPINA

#endif /* FILTERS_FILLHOLES2DFILTER_H_ */
