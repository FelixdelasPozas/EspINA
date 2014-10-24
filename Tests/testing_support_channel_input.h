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

#ifndef ESPINA_TESTING_SUPPORT_CHANNEL_INPUT_H
#define ESPINA_TESTING_SUPPORT_CHANNEL_INPUT_H

#include <Core/Analysis/Input.h>
#include <Core/Analysis/Filter.h>

namespace ESPINA {
  namespace Testing {
    class DummyChannelReader
    : public Filter
    {
    public:
      explicit DummyChannelReader();

      virtual void restoreState(const State& state) override {}
      virtual State state() const                   override {return State();}
    protected:
      virtual Snapshot saveFilterSnapshot() const   override {return Snapshot();}
      virtual bool needUpdate() const               override {return false;}
      virtual bool needUpdate(Output::Id id) const  override {return false;}
      virtual void execute()                        override {}
      virtual void execute(Output::Id id)           override {}
      virtual bool ignoreStorageContent() const     override {return false;}
    };

    InputSPtr channelInput();
  }
}

#endif // ESPINA_TESTING_SUPPORT_CHANNEL_INPUT_H
