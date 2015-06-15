/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2015  <copyright holder> <email>
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

#ifndef ESPINA_VIEW_UTILS_H
#define ESPINA_VIEW_UTILS_H

#include <Core/Utils/Spatial.h>
#include <vtkSmartPointer.h>

class vtkImageData;

namespace ESPINA
{
  namespace GUI
  {
    namespace View
    {
      namespace Utils
      {
        void addPadding(vtkSmartPointer<vtkImageData> image, int normal);

        template<typename T>
        void repositionActor(T actor, Nm depth, int normal)
        {
          double pos[3];

          actor->GetPosition(pos);

          pos[normal] += depth;

          actor->SetPosition(pos);
          actor->Modified();
        }
      }
    }
  }
}

#endif // ESPINA_VIEW_UTILS_H
