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

#ifndef ESPINA_SUPPORT_UTILS_TAG_UTILS_H
#define ESPINA_SUPPORT_UTILS_TAG_UTILS_H

#include <GUI/Model/SegmentationAdapter.h>

class QUndoStack;

namespace ESPINA
{
  class ModelFactory;

  namespace Support
  {
    namespace Utils
    {
      namespace Tags
      {
        /** \brief Helper method to show a dialog to manage the tags of a group of segmentations
         * \param[in] segmentations list of segmentations to manage.
         * \param[in] undoStack application QUndoStack object.
         * \param[in] factory model factory to create tags extesnions.
         *
         */
        void manageTags(SegmentationAdapterList segmentations, QUndoStack *undoStack, ModelFactory *factory);
      }
    }
  }
}

#endif // ESPINA_SUPPORT_UTILS_TAGUTILS_H
