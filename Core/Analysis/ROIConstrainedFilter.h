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

#ifndef ROI_CONSTRAINED_H_
#define ROI_CONSTRAINED_H_

#include <Core/Analysis/Data/Volumetric/ROI.h>

namespace ESPINA
{
  class ROIConstrained
  {
    public:
      /* \brief ROIConstraied class virtual destructor.
       *
       */
      virtual ~ROIConstrained()
      {}

      /* \brief Set region of interest of the element.
       *
       */
      virtual void setROI(ROISPtr roi) = 0;

      /* \brief Get region of interest affecting the element.
       *
       */
      virtual ROISPtr ROI() = 0;
  };
}

#endif // ROI_CONSTRAINED_H_
