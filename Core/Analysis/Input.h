/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2014  <copyright holder> <email>
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

#ifndef ESPINA_INPUT_H
#define ESPINA_INPUT_H

#include "Output.h"

namespace EspINA {

  class Input
  {
  public:
    explicit Input(FilterSPtr filter, OutputSPtr output);

    FilterSPtr filter() const
    { return m_filter; }

    OutputSPtr output() const
    { return m_output; }

  private:
    FilterSPtr m_filter;
    OutputSPtr m_output;
  };

  using InputSPtr  = std::shared_ptr<Input>;
  using InputSList = QList<InputSPtr>;

  InputSPtr   getInput(FilterSPtr filter, Output::Id id);
  InputSList getInputs(FilterSPtr filter);
}

#endif // ESPINA_INPUT_H
