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

#ifndef IVOI_H
#define IVOI_H

#include "EspinaGUI_Export.h"

#include "ITool.h"

namespace EspINA
{
  class EspinaGUI_EXPORT IVOI
  : public ITool
  {
  public:
    typedef double * Region;

    virtual Region region() = 0;
  };

  typedef boost::shared_ptr<IVOI> IVOISPtr;

} // namespace EspINA

#endif // IVOI_H