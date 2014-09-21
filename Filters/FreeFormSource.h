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

#ifndef ESPINA_FREE_FORM_SOURCE_H
#define ESPINA_FREE_FORM_SOURCE_H

#include "Filters/EspinaFilters_Export.h"

// ESPINA
#include <Core/Analysis/Filter.h>
#include <Core/Utils/BinaryMask.hxx>

namespace ESPINA
{
  class EspinaFilters_EXPORT FreeFormSource
  : public Filter
  {
    public:
  		/** brief FreeFormSource class constructor.
			 * \param[in] inputs, list of input smart pointers.
			 * \param[in] type, FreeFormSource type.
			 * \param[in] scheduler, scheduler smart pointer.
			 *
  		 */
      explicit FreeFormSource(InputSList inputs,
                              Filter::Type     type,
                              SchedulerSPtr    scheduler);

      /** brief FreeFormSource class virtual destructor.
       *
       */
      virtual ~FreeFormSource();

      /** brief Implements Persistent::restoreState().
       *
       */
      virtual void restoreState(const State &state);

      /** brief Implements Persistent::state().
       *
       */
      virtual State state() const;

      /** brief Sets the mask that serves as input for the output volume.
       *
       */
      void setMask(BinaryMaskSPtr<unsigned char> mask)
      { m_mask = mask; }

    protected:
      /** brief Implements Filter::saveFilterSnapshot().
       *
       */
      virtual Snapshot saveFilterSnapshot() const;

      /** brief Implements Filter::needUpdate().
       *
       */
      virtual bool needUpdate() const
      { return needUpdate(0); }

      /** brief Implements Filter::needUpdate(id).
       *
       */
      virtual bool needUpdate(Output::Id oId) const;

      /** brief Implements Filter::execute().
       *
       */
      virtual void execute()
      { execute(0); }

      /** brief Implements Filter::execute(id).
       *
       */
      virtual void execute(Output::Id oId);

      /** brief Implements Filter::ignoreStorageContent().
       *
       */
      virtual bool ignoreStorageContent() const
      { return false; }

      /** brief Implements Filter::invalidateEditedRegions().
       *
       */
      virtual bool invalidateEditedRegions();

    private:
      BinaryMaskSPtr<unsigned char> m_mask;
  };

} // namespace ESPINA


#endif // ESPINA_FREE_FORM_SOURCE_H
