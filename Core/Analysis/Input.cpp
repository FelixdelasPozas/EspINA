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

#include "Input.h"
#include "Filter.h"

using namespace EspINA;

//----------------------------------------------------------------------------
Input::Input(FilterSPtr filter, OutputSPtr output)
: m_filter{filter}
, m_output{output}
{
  Q_ASSERT(filter.get() == output->filter());
}

//----------------------------------------------------------------------------
InputSPtr EspINA::getInput(FilterSPtr filter, Output::Id id)
{
  return InputSPtr{new Input(filter, filter->output(id))};
}

//----------------------------------------------------------------------------
InputSList EspINA::getInputs(FilterSPtr filter)
{
  InputSList outputs;

  for(auto output : filter->outputs())
  {
    outputs << InputSPtr{new Input(filter, output)};
  }

  return outputs;
}