/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  <copyright holder> <email>

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

#ifndef ESPINA_VOI_TOOL_H
#define ESPINA_VOI_TOOL_H

#include "Support/Tool.h"
#include <GUI/Selectors/Selector.h>

namespace EspINA
{
  using VOI = std::shared_ptr<Selector::SelectionMask>;

  class EspinaSupport_EXPORT VOITool
  : public Tool
  {
  public:
    typedef double * Region;

    virtual VOI currentVOI() = 0;
  };

  using VOIToolSPtr = std::shared_ptr<VOITool>;
} // namespace EspINA

#endif // IVOI_H
