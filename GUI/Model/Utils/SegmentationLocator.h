/*

 Copyright (C) 2017 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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

#ifndef GUI_MODEL_UTILS_SEGMENTATIONLOCATOR_H_
#define GUI_MODEL_UTILS_SEGMENTATIONLOCATOR_H_

#include <GUI/EspinaGUI_Export.h>

// ESPINA
#include <GUI/Model/Utils/DBVH.h>

// C++
#include <memory>

namespace ESPINA
{
  namespace GUI
  {
    namespace Model
    {
      namespace Utils
      {
        /** \class SegmentationLocator
         * \brief Implements the basic API of DBVHNode class for locating segmentations in 3D space.
         *
         * NOTE: this class relies on the DBVHNode maintained by the model adapter. For custom views without all the
         * segmentation sources use the ManualSegmentationLocator class as this class won't support the insertion or
         * removal of segmentations from the DBVH tree.
         *
         */
        class EspinaGUI_EXPORT SegmentationLocator
        {
          public:
            /** \brief SegmentationLocator class constructor.
             * \param[in] dbvh DBVH tree root node.
             *
             */
            explicit SegmentationLocator(DBVHNode *dbvh);

            /** \brief SegmentationLocator class virtual destructor.
             *
             */
            virtual ~SegmentationLocator()
            {}

            /** \brief Returns the list of elements whose bounds contains the given point.
             * \param[in] point Point 3D coordinates.
             * \param[in] spacing Scene spacing.
             *
             */
            const ViewItemAdapterSList contains(const NmVector3 &point, const NmVector3 &spacing = NmVector3{1,1,1}) const;

            /** \brief Returns the list of elements whose bounds intersects the given one.
             * \param[in] bounds Bounds object.
             * \param[in] spacing Scene spacing.
             *
             */
            const ViewItemAdapterSList intersects(const Bounds &bounds, const NmVector3 &spacing = NmVector3{1,1,1}) const;

          protected:
            DBVHNode *m_dbvh; /** DBVH tree root node. */
        };

        /** \class ManualSegmentationLocator
         * \brief Wrapper around DBVHNode class to use in views with manual sources.
         *
         *
         */
        class EspinaGUI_EXPORT ManualSegmentationLocator
        : public SegmentationLocator
        {
          public:
            /** \brief ManualSegmentationLocator class constructor.
             *
             */
            explicit ManualSegmentationLocator();

            /** \brief ManualSegmentationLocator class virtual destructor.
             *
             */
            virtual ~ManualSegmentationLocator();

            /** \brief Inserts the given list of elements into the tree.
             * \param[in] elements List of elements.
             *
             */
            void insert(const ViewItemAdapterSList &elements);

            /** \brief Removes the given list of elements from the tree.
             * \param[in] elements List of elements.
             *
             */
            void remove(const ViewItemAdapterSList &elements);

            /** \brief Inserts the given element into the tree.
             * \param[in] element Element item.
             *
             */
            void insert(const ViewItemAdapterSPtr element);

            /** \brief Removes an element from the node
             *
             */
            void remove(ViewItemAdapterSPtr element);
        };
      } // namespace Utils
    } // namespace Model
  } // namespace GUI

  using SegmentationLocatorSPtr = std::shared_ptr<GUI::Model::Utils::SegmentationLocator>;

} // namespace ESPINA

#endif // GUI_MODEL_UTILS_SEGMENTATIONLOCATOR_H_
