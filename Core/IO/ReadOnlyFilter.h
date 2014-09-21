/*
 *
 * Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 * This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
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

#ifndef ESPINA_READONLYFILTER_H
#define ESPINA_READONLYFILTER_H

#include "Core/EspinaCore_Export.h"

// ESPINA
#include <Core/Analysis/Filter.h>

namespace ESPINA {

  namespace IO {

    namespace SegFile {

      class EspinaCore_EXPORT ReadOnlyFilter
      : public Filter
      {
      public:
      	/** brief ReadOnlyFilter class constructor.
      	 * \param[in] inputs, list of input smart pointers.
      	 * \param[in] type, filter type.
      	 *
      	 */
        explicit ReadOnlyFilter(InputSList inputs, Type type)
        : Filter(inputs, type, SchedulerSPtr())
        {}

      	/** brief Implements Persistent::restoreState().
      	 *
      	 */
        virtual void restoreState(const State& state)
        { m_state = state; }

      	/** brief Implements Persistent::state().
      	 *
      	 */
        virtual State state() const
        { return m_state; }

      protected:
      	/** brief Implements Filter::saveFilterSnapshot().
      	 *
      	 */
        virtual Snapshot saveFilterSnapshot() const
        {return Snapshot(); }

      	/** brief
      	 *
      	 */
        virtual bool needUpdate() const
        { return m_outputs.isEmpty();}

      	/** brief Implements Filter::needUpdate().
      	 *
      	 */
        virtual bool needUpdate(Output::Id id) const
        { return !m_outputs.contains(id) || !m_outputs[id]->isValid(); }

      	/** brief Implements Filter::execute().
      	 *
      	 */
        virtual void execute()
        {}

      	/** brief Implements Filter::execute(id).
      	 *
      	 */
        virtual void execute(Output::Id id)
        {}

      	/** brief Implements Filter::ignoreStorageContent().
      	 *
      	 */
        virtual bool ignoreStorageContent() const
        { return false; }

      	/** brief Implements Filter::invalidateEditedRegions().
      	 *
      	 */
        virtual bool invalidateEditedRegions()
        {return false;}

      private:
        State m_state;
      };
    }
  }

}

#endif // ESPINA_READONLYFILTER_H
