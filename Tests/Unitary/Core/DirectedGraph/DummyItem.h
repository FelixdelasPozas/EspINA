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

#ifndef ESPINA_TESTS_DUMMYITEM_H
#define ESPINA_TESTS_DUMMYITEM_H

#include <Core/Analysis/AnalysisItem.h>
#include <memory>

namespace UnitTesting {

  class DummyItem 
  : public EspINA::AnalysisItem
  {
  public:
    virtual void restoreState(const EspINA::State& state){}
    virtual EspINA::State saveState() const{}
  };

  using DummyItemSPtr = std::shared_ptr<DummyItem>;
}
#endif // ESPINA_TESTS_DUMMYITEM_H
