/*

    Copyright (C) 2014
    Jorge Pe�a Pastor<jpena@cesvima.upm.es>,
    Felix de las Pozas<fpozas@cesvima.upm.es>

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

#ifndef ESPINA_MODEL_ADAPTER_UTILS_H
#define ESPINA_MODEL_ADAPTER_UTILS_H

#include "GUI/EspinaGUI_Export.h"

// ESPINA
#include <GUI/Model/ModelAdapter.h>

namespace ESPINA
{
  namespace GUI
  {
    namespace Model
    {
      namespace Utils
      {
        /** \brief Returns the first number for a segmentation not used in an analysis.
         * \param[in] model analysis model adapter
         *
         */
        unsigned int EspinaGUI_EXPORT firstUnusedSegmentationNumber(const ModelAdapterSPtr model);

        struct Items
        {
          ViewItemAdapterList stacks;
          ViewItemAdapterList segmentations;
        };

        /** \brief Classifies items into segmentations and channels.
         * \param[in] items items to classify.
         *
         */
        Items EspinaGUI_EXPORT classifyViewItems(const ViewItemAdapterList &items);

        /** \brief Classifies items belonging to the subgroup into segmentations and channels.
         * \param[in] items items to classify.
         * \param[in] group group of items.
         *
         */
        Items EspinaGUI_EXPORT classifyViewItems(const ViewItemAdapterList &items, const Items &group);

      } // namespace Util
    } // namespace Model
  } // namespace GUI
} // namespace ESPINA

#endif // ESPINA_MODEL_ADAPTER_UTILS_H
