/*
    
    Copyright (C) 2014 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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

#ifndef ESPINA_VTKPOLYDATAUTILS_H_
#define ESPINA_VTKPOLYDATAUTILS_H_

// VTK
#include <vtkSmartPointer.h>

class vtkPolyData;
class QByteArray;
class QString;

namespace ESPINA
{
  namespace PolyDataUtils
  {
    struct IO_Error_Exception{};

    QByteArray savePolyDataToBuffer(const vtkSmartPointer<vtkPolyData> polydata) throw (IO_Error_Exception);

    vtkSmartPointer<vtkPolyData> readPolyDataFromFile(QString fileName) throw (IO_Error_Exception);
  }
}

#endif /* ESPINA_VTKPOLYDATAUTILS_H_ */
