/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2015  Jorge Peña Pastor <jpena@cesvima.upm.es>
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

#ifndef ESPINA_REPRESENTATION_UTILS_H
#define ESPINA_REPRESENTATION_UTILS_H

#include <GUI/Representations/RepresentationState.h>
#include <GUI/Representations/RepresentationPool.h>
#include <Support/Representations/RepresentationFactory.h>

namespace ESPINA
{
  namespace RepresentationUtils
  {
    Plane plane(const RepresentationState &state);

    void setPlane(RepresentationState &state, const Plane plane);

    void setPlane(RepresentationPoolSPtr pool, const Plane plane);

    Nm segmentationDepth(const RepresentationState &state);

    void setSegmentationDepth(RepresentationState &state, const Nm depth);

    void setSegmentationDepth(RepresentationPoolSPtr pool, const Nm depth);
  }

  namespace Support
  {
    namespace Representations
    {
      namespace Utils
      {
        const RepresentationGroup CHANNELS_GROUP      = "Channel";
        const RepresentationGroup SEGMENTATIONS_GROUP = "Segmentation";
      }
    }
  } // namespace Support
} // namespace ESPINA

#endif // ESPINA_REPRESENTATION_UTILS_H
