/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>

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

#ifndef ESPINA_ROI_PROVIDER_H
#define ESPINA_ROI_PROVIDER_H

#include <Core/Analysis/Data/Volumetric/ROI.h>

namespace ESPINA {

  class ROIProvider
  {
  public:
    virtual ~ROIProvider() {}

    /** \brief Returns current ROI reference
     *
     *  After this operation the ROI is still available from the ROI provider
     */
    virtual ROISPtr currentROI() = 0;

    /** \brief Release the current ROI
     *
     *  After this operation the ROI won't be available from the ROI provider
     */
    virtual void consumeROI() = 0;
  };

  using ROIProviderPtr = ROIProvider *;

} // namespace ESPINA

#endif // ESPINA_ROI_PROVIDER_H
