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

#include "GUI/EspinaGUI_Export.h"

// ESPINA
#include <Core/Utils/Spatial.h>

// VTK
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
        /** \brief Adds a padding border to the given image except on the given dimension.
         * \param[in] image image data.
         * \param[in] normal normal of the axis that won't be needing padding.
         *
         */
        void EspinaGUI_EXPORT addPadding(vtkSmartPointer<vtkImageData> image, int normal);

        /** \brief Repositions the given actor depth units in the normal direction.
         * \param[in] actor actor object.
         * \param[in] depth distance to reposition.
         * \param[in] normal normal of the axis of movement.
         *
         */
        template<typename T>
        void EspinaGUI_EXPORT repositionActor(T actor, Nm depth, int normal)
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
