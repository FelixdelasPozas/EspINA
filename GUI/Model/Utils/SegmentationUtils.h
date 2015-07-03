/*
 * Copyright 2015 <copyright holder> <email>
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

#ifndef ESPINA_GUI_MODEL_UTILS_SEGMENTATION_UTILS_H
#define ESPINA_GUI_MODEL_UTILS_SEGMENTATION_UTILS_H

#include "GUI/Model/SegmentationAdapter.h"

namespace ESPINA
{
  namespace GUI
  {
    namespace Model
    {
      namespace Utils
      {
        /** \brief Returns the segmentation adapter smart pointer from the item adapter raw pointer.
         * \param[in] item item adapter raw pointer.
         */
        SegmentationAdapterPtr EspinaGUI_EXPORT segmentationPtr(ItemAdapterPtr item);

        /** \brief Returns true if the given item is a segmentation item.
         * \param[in] item item adapter raw pointer.
         *
         */
        bool EspinaGUI_EXPORT isSegmentation(ItemAdapterPtr item);

        /** \brief Returns the equivalent list of View Item Adapter Pointers
         *
         */
        ViewItemAdapterList EspinaGUI_EXPORT toViewItemList(SegmentationAdapterList segmentations);

        /** \brief Returns the equivalent list of View Item Adapter Pointers
         *
         */
        ViewItemAdapterList EspinaGUI_EXPORT toViewItemList(SegmentationAdapterSList segmentations);

        /** \brief Returns the equivalent list of View Item Adapter Smart Pointers
         *
         */
        ViewItemAdapterSList EspinaGUI_EXPORT toViewItemSList(SegmentationAdapterSList segmentations);

        //void addIfNotContained(ItemAdapterSList)
      }
    }
  }
}

#endif // ESPINA_GUI_MODEL_UTILS_SEGMENTATION_UTILS_H
