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

#ifndef ESPINA_TESTS_DUMMYITEM_H
#define ESPINA_TESTS_DUMMYITEM_H

#include <Core/Analysis/Persistent.h>
#include <memory>

namespace UnitTesting {

  class DummyItem 
  : public ESPINA::Persistent
  {
  public:
    virtual void restoreState(const ESPINA::State& state){}
    virtual ESPINA::State state() const { return ESPINA::State(); }
    virtual ESPINA::Snapshot snapshot() const {return ESPINA::Snapshot();}
    virtual void unload(){}
  };

  using DummyItemSPtr = std::shared_ptr<DummyItem>;
}
#endif // ESPINA_TESTS_DUMMYITEM_H
