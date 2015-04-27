/*
 * Copyright 2015  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#ifndef ESPINA_SUPPORT_ROI_ACCUMULATOR_H
#define ESPINA_SUPPORT_ROI_ACCUMULATOR_H

#include <Core/Analysis/Data/Volumetric/ROI.h>
#include "ROIProvider.h"

namespace ESPINA
{
  namespace Support
  {
    class ROIAccumulator
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
