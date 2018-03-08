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

#include <Support/EspinaSupport_Export.h>

// ESPINA
#include <GUI/Representations/RepresentationState.h>
#include <GUI/Representations/RepresentationPool.h>
#include <Support/Representations/RepresentationFactory.h>

// C++
#include <memory>

// Qt
#include <QList>

namespace ESPINA
{
  namespace Support
  {
    namespace Representations
    {
      namespace Utils
      {
        const RepresentationGroup STACKS_GROUP        = "Stack";
        const RepresentationGroup SEGMENTATIONS_GROUP = "Segmentation";

        /** \brief Returns true if the given representation is for stacks.
         * \param[in] representation Representation struct.
         *
         */
        inline bool isStackRepresentation(const Representation &representation)
        { return STACKS_GROUP == representation.Group; }

        /** \brief Returns true if the given representation is for segmentations.
         * \param[in] representation Representation struct.
         *
         */
        inline bool isSegmentationRepresentation(const Representation &representation)
        { return SEGMENTATIONS_GROUP == representation.Group; }

        /** \brief Returns a list of settings of the templated type present in the given context.
         * \param[in] context Application context data.
         *
         */
        template<class T>
        PoolSettingsSList getPoolSettings(const Support::Context & context)
        {
          PoolSettingsSList result;

          for (auto representation : context.representations())
          {
            for (auto settings : representation.Settings)
            {
              if (std::dynamic_pointer_cast<T>(settings)) result << settings;
            }
          }

          return result;
        }
      }
    }
  } // namespace GUI
} // namespace ESPINA

#endif // ESPINA_REPRESENTATION_UTILS_H
