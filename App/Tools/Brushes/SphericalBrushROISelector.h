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

#ifndef ESPINA_SPHERICAL_BRUSH_ROI_SELECTOR_H_
#define ESPINA_SPHERICAL_BRUSH_ROI_SELECTOR_H_

// ESPINA
#include "ROISelectorBase.h"

// Qt
#include <QObject>

namespace ESPINA
{
  class SphericalBrushROISelector
  : public ROISelectorBase
  {
    Q_OBJECT
    public:
      /** \brief SphericalBrushROISelector class constructor.
       *
       */
      explicit SphericalBrushROISelector();

    protected slots:
			/** \brief Implements BrushSelector::createBrushShape().
			 *
			 */
      virtual BrushSelector::BrushShape createBrushShape(ViewItemAdapterPtr item,
                                                         NmVector3 center,
                                                         Nm radius,
                                                         Plane plane);
  };

  using SphericalBrushROISelectorSPtr = std::shared_ptr<SphericalBrushROISelector>;

} // namespace ESPINA

#endif // ESPINA_SPHERICAL_BRUSH_ROI_SELECTOR_H_
