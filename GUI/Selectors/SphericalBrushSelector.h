/*

    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>

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

#ifndef ESPINA_SPHERICAL_BRUSH_SELECTOR_H
#define ESPINA_SPHERICAL_BRUSH_SELECTOR_H

#include "GUI/EspinaGUI_Export.h"

// ESPINA
#include <GUI/Selectors/BrushSelector.h>

namespace ESPINA
{
  class EspinaGUI_EXPORT SphericalBrushSelector
  : public BrushSelector
  {
    Q_OBJECT
    public:
      /** brief SphericalBrushSelector class constructor.
       *
       */
      explicit SphericalBrushSelector();

    protected slots:
      /** brief Implements BrushSelector::createBrushShape().
       *
       */
      virtual BrushSelector::BrushShape createBrushShape(ViewItemAdapterPtr item,
                                                         NmVector3 center,
                                                         Nm radius,
                                                         Plane plane);
  };

  using SphericalBrushSelectorSPtr = std::shared_ptr<SphericalBrushSelector>;

} // namespace ESPINA

#endif // ESPINA_SPHERICAL_BRUSH_SELECTOR_H
