/*

    Copyright (C) 2016 Felix de las Pozas Alvarez<fpozas@cesvima.upm.es>

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

#ifndef ESPINA_GUI_MODEL_UTILS_SEGMENTATION_UTILS_H
#define ESPINA_GUI_MODEL_UTILS_SEGMENTATION_UTILS_H

#include "GUI/EspinaGUI_Export.h"

// ESPINA
#include <GUI/Model/ModelAdapter.h>
#include <GUI/Model/SegmentationAdapter.h>

// VTK
#include <vtkSmartPointer.h>

class vtkPolyData;

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

        /** \brief Returns the segmentation adapter smart pointer from the item adapter raw pointer.
         * \param[in] item item adapter raw pointer.
         */
        ConstSegmentationAdapterPtr EspinaGUI_EXPORT segmentationPtr(ConstItemAdapterPtr item);

        /** \brief Returns true if the given item is a segmentation item.
         * \param[in] item item adapter raw pointer.
         *
         */
        bool EspinaGUI_EXPORT isSegmentation(ItemAdapterPtr item);

        /** \brief Returns the categorical name of the segmentation.
         * \param[in] segmentation item adapter.
         *
         */
        const QString EspinaGUI_EXPORT categoricalName(SegmentationAdapterSPtr segmentation);

        /** \brief Returns the categorical name of the segmentation.
         * \param[in] segmentation item adapter.
         *
         */
        const QString EspinaGUI_EXPORT categoricalName(SegmentationAdapterPtr segmentation);

        /** \brief Returns the connections the given polydata has with the segmentations of the given model. The connections
         *  returned are not in the model, just the ones looking at the terminal nodes of the polydata.
         * \param[in] polyData skeleton polydata.
         * \param[in] model session model adapter.
         *
         * NOTE: as the only information is the polydata, the connection list has no "item1", it must be filled by the caller.
         *
         */
        ConnectionList EspinaGUI_EXPORT connections(vtkSmartPointer<vtkPolyData> polyData, const ModelAdapterSPtr model);

        /** \brief Returns the axon connection of the given synapse or nullptr if the synapse is not yet connected to an axon.
         * \param[in] synapse Synapse category segmentation raw pointer.
         *
         */
        SegmentationAdapterSPtr EspinaGUI_EXPORT axonOf(const SegmentationAdapterPtr synapse);

        /** \brief Returns the dendrite connection of the given synapse or nullptr if the synapse is not yet connected to a dendrite.
         * \param[in] synapse Synapse category segmentation raw pointer.
         *
         */
        SegmentationAdapterSPtr EspinaGUI_EXPORT dendriteOf(const SegmentationAdapterPtr synapse);

        /** \brief Returns the concatenation of segmentation names separated by commas and 'and' for last
         * item.
         * \param[in] list List of segmentations.
         *
         */
        const QString EspinaGUI_EXPORT segmentationListNames(const SegmentationAdapterList &list);

        /** \brief Returns the concatenation of segmentation names separated by commas and 'and' for last
         * item.
         * \param[in] list List of segmentations.
         *
         */
        const QString EspinaGUI_EXPORT segmentationListNames(const SegmentationAdapterSList &list);
      }
    }
  }
}

#endif // ESPINA_GUI_MODEL_UTILS_SEGMENTATION_UTILS_H
