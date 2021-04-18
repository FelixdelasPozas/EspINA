/*
 *    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 *    This file is part of ESPINA.
 *
 *    ESPINA is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ESPINA_FILTERS_UTILS_STENCIL_H
#define ESPINA_FILTERS_UTILS_STENCIL_H

#include "Filters/EspinaFilters_Export.h"

// VTK
#include <vtkSmartPointer.h>

class vtkPlane;
class vtkImageStencilData;

namespace ESPINA
{
  class VolumeBounds;

  namespace Filters
  {
    namespace Utils
    {
      namespace Stencil
      {
        /** \brief Creates a stencil data image with the given bounds from a given plane definition.
         * \param[in] plane plane definition.
         * \param[in] bounds output image bounds.
         *
         */
        vtkSmartPointer<vtkImageStencilData> EspinaFilters_EXPORT fromPlane(vtkSmartPointer<vtkPlane> plane, const VolumeBounds &bounds);
      };
    }
  }
}

#endif // ESPINA_FILTERS_UTILS_STENCIL_H
