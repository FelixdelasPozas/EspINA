/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  Jorge PeÃ±a Pastor <email>

    This program is free software: you can redistribute it and/or modify
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


#ifndef CLOSINGFILTER_H
#define CLOSINGFILTER_H

#include "EspinaCore_Export.h"

#include "MorphologicalEditionFilter.h"

namespace EspINA
{

class EspinaCore_EXPORT ClosingFilter
: public MorphologicalEditionFilter
{

public:
  explicit ClosingFilter(NamedInputs inputs,
                         Arguments   args,
                         FilterType  type);
  virtual ~ClosingFilter();

protected:
  virtual void run(FilterOutputId oId);
};

} // namespace EspINA


#endif // CLOSINGFILTER_H
