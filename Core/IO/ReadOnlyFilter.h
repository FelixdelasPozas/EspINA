/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2013  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 * This program is free software: you can redistribute it and/or modify
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

#include <Core/Analysis/Filter.h>

namespace EspINA {

  namespace IO {

    namespace SegFile {

      class EspinaCore_EXPORT ReadOnlyFilter
      : public Filter
      {
      public:
        explicit ReadOnlyFilter(OutputSList inputs, Type type)
        : Filter(inputs, type, SchedulerSPtr()) {}

        virtual void restoreState(const State& state)
        { m_state = state; }

        virtual State state() const
        { return m_state; }

      protected:
        virtual Snapshot saveFilterSnapshot() const {return Snapshot(); }

        virtual bool needUpdate() const
        { return m_outputs.isEmpty();}

        virtual bool needUpdate(Output::Id id) const
        {
          for(unsigned int i = m_outputs.size(); i <= id; ++i)
          {
            m_outputs << OutputSPtr{new Output(this, i)};
          }
          return !m_outputs[id]->isValid(); 
        }

        virtual void execute()
        {}

        virtual void execute(Output::Id id)
        {}

        virtual bool ignoreStorageContent() const
        { return false; }

        virtual bool invalidateEditedRegions()
        {return false;}

      private:
        State m_state;
      };
    }
  }

}

#endif // ESPINA_READONLYFILTER_H
