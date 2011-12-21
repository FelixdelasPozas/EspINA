/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2011  Jorge Peña Pastor <jpena@cesvima.upm.es>

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

//----------------------------------------------------------------------------
// File:    DynamicWidget.h
// Purpose: Define the interface for every widget which depends on the current
//          LOD and activity
//----------------------------------------------------------------------------
#ifndef DYNAMICWIDGET_H
#define DYNAMICWIDGET_H

class DynamicWidget
{
public:
  virtual void increaseLOD() = 0;
  virtual void decreaseLOD() = 0;
  virtual void setLOD() = 0;
  virtual void setActivity() = 0;
};

#endif // DYNAMICWIDGET_H
