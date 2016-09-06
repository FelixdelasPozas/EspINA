/*

 Copyright 2015  Jorge Peña Pastor <jpena@cesvima.upm.es>

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

#ifndef ESPINA_SUPPORT_ROI_ACCUMULATOR_H
#define ESPINA_SUPPORT_ROI_ACCUMULATOR_H

#include <Support/EspinaSupport_Export.h>

// ESPINA
#include "ROIProvider.h"
#include <Core/Analysis/Data/Volumetric/ROI.h>

namespace ESPINA
{
  namespace Support
  {
    class EspinaSupport_EXPORT ROIAccumulator
    {
    public:
      explicit ROIAccumulator();

      void setProvider(ROIProviderPtr provider);

      /** \brief Returns current ROI reference
       *
       *  After this operation the ROI is still available from the ROI provider
       */
      ROISPtr currentROI();

      operator ROISPtr() const;

      /** \brief Release the current ROI
       *
       *  After this operation the ROI won't be available from the ROI provider
       */
      void clear();

    private:
      ROIProviderPtr m_provider;
    };

    using ROIAccumulatorSPtr = std::shared_ptr<ROIAccumulator>;
  }
}

#endif // ESPINA_SUPPORT_ROIACCUMULATOR_H
