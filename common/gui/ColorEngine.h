/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  Jorge Peña Pastor <jpena@cesvima.upm.es>

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


#ifndef COLORENGINE_H
#define COLORENGINE_H
#include <QColor>

class Segmentation;
class vtkSMProxy;

class ColorEngine
{
public:
  virtual QColor color(Segmentation *seg) = 0;
  virtual vtkSMProxy *lut(Segmentation *seg) = 0;
};

#endif // COLORENGINE_H
