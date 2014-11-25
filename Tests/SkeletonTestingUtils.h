/*

    Copyright (C) 2014 Félix de las Pozas Álvarez <fpozas@cesvima.upm.es>

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

#ifndef ESPINA_TESTING_SKELETON_TESTING_UTILS_H
#define ESPINA_TESTING_SKELETON_TESTING_UTILS_H

#include "vtkSmartPointer.h"
#include "vtkPolyData.h"

namespace ESPINA {
  namespace Testing {

    // number of nodes must be >= 2
    vtkSmartPointer<vtkPolyData> createRandomTestSkeleton(int numberOfNodes = 2);

    vtkSmartPointer<vtkPolyData> createSimpleTestSkeleton();

  } // Testing
} // ESPINA

#endif // ESPINA_TESTING_SKELETON_TESTING_UTILS_H
