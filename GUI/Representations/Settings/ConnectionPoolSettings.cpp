/*

 Copyright (C) 2018 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

 This file is part of ESPINA.

 ESPINA is free software: you can redistribute it and/or modify
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

#include <GUI/Representations/Settings/ConnectionPoolSettings.h>

using namespace ESPINA::GUI::Representations::Settings;

const QString ConnectionPoolSettings::CONNECTION_SIZE = "ConnectionSize";

//--------------------------------------------------------------------
ConnectionPoolSettings::ConnectionPoolSettings()
{
  set<int>(CONNECTION_SIZE, 4);
}

//--------------------------------------------------------------------
void ConnectionPoolSettings::setConnectionSize(int size)
{
  if(size != connectionSize())
  {
    set<int>(CONNECTION_SIZE, size);
  }
}

//--------------------------------------------------------------------
const int ConnectionPoolSettings::connectionSize() const
{
  return get<int>(CONNECTION_SIZE);
}
