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

#ifndef GUI_MODEL_UTILS_DBVH_H_
#define GUI_MODEL_UTILS_DBVH_H_

#include <GUI/EspinaGUI_Export.h>

// ESPINA
#include <GUI/Model/ViewItemAdapter.h>
#include <Core/Utils/EspinaException.h>
#include <Core/Utils/Spatial.h>
#include <Core/Utils/Bounds.h>
#include <Core/Utils/EspinaException.h>

// C++
#include <functional>
#include <memory>

class QFile;

namespace ESPINA
{
  namespace GUI
  {
    namespace Model
    {
      namespace Utils
      {
        /** \brief lessThan performs ordering templated over axis direction and element type. Element must provide method "output->bounds()".
         * \param[in] lhs left object.
         * \param[in] rhs right object.
         *
         */
        template<typename T, int I> bool lessThan(const T lhs, const T rhs)
        {
          auto left  = lhs->output()->bounds();
          auto right = rhs->output()->bounds();

          if(left[2*I] <= right[2*I]) return true;

          return false;
        }

        /** \brief Returns the bounding box of the given elements. Element must provide method "output->bounds()".
         * \param[in] elements List of elements.
         *
         */
        template<typename T> const Bounds AABB(const QList<T> &elements)
        {
          Bounds result;

          if(!elements.isEmpty())
          {
            result = elements.first()->output()->bounds();

            for(auto element: elements)
            {
              result = boundingBox(result, element->output()->bounds());
            }
          }

          return result;
        }

        /** \brief Returns the best split method for the given bounds.
         * \param[in] bounds Bounds object.
         *
         */
        template<typename T> std::function<bool(const T &lhs, const T &rhs)> splitMethod(const Axis axis)
        {
          switch(idx(axis))
          {
            case 0: return lessThan<T,0>;
            case 1: return lessThan<T,1>;
            case 2: return lessThan<T,2>;
            default:
            {
              auto message = QObject::tr("Wrong Axis parameter: %1.").arg(toText(axis));
              auto details = QObject::tr("ESPINA::splitMethod(bounds, axis) -> ") + message;
              throw Core::Utils::EspinaException(message, details);
            }
            break;
          }
         }

        /** \class DBVHNode
         * \brief Node for the dynamic bounding volume hierarchy tree. Based on the paper
         * "Fast, Effective BVH Updates for Animated Scenes" by Daniel Kopta, Thiago Ize,
         * Josef Spjut, Erik Brunvand, Al Davis & Andrew Kensler.
         *
         */
        class EspinaGUI_EXPORT DBVHNode
        : public QObject
        {
            Q_OBJECT
          public:
            /** \brief DBVHNode class constructor.
             * \param[in] parent parent node of this one.
             * \param[in] depth Depth of the node for debugging purposes.
             * \param[in] elements List of elements
             *
             */
            explicit DBVHNode(DBVHNode *parent = nullptr, const unsigned int depth = 0, const ViewItemAdapterSList &elements = ViewItemAdapterSList());

            /** \brief DBVHNode class virtual destructor.
             *
             */
            virtual ~DBVHNode();

            /** \brief Prints the node structure, for debugging purposes.
             * \param[in] indentationDepth indentation depth.
             *
             */
            void debugInformation(QFile *file = nullptr) const;

            /** \brief Returns true if the node is a leaf and false otherwise.
             *
             */
            const bool isLeaf() const
            { return (m_element != nullptr); }

            /** \brief Returns true if the node is the root node and false otherwise.
             *
             */
            const bool isRoot() const
            { return (m_parent == nullptr); }

            /** \brief Returns the depth of the node.
             *
             */
            const unsigned int depth() const
            { return m_depth; }

            /** \brief Returns the axis aligned bounding box of the contained elements.
             *
             */
            const Bounds &AABB() const
            { return m_bounds; }

            /** \brief Returns the number of nodes of the subtree (node or subnodes).
             *
             */
            const int size() const
            { return m_size; }

            /** \brief Returns the list of contained elements.
             *
             */
            const ViewItemAdapterSList elements() const;

            /** \brief Removes all data from the node.
             *
             */
            void clear();

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
            bool remove(ViewItemAdapterSPtr element);

            /** \brief Rebuilds the node.
             *
             */
            void rebuild();

            /** \brief Optimizes the subtree of the node.
             *
             */
            void optimize();

            /** \brief Returns the list of elements whose bounds contains the given point.
             * \param[in] point Point 3D coordinates.
             * \param[in] spacing View's spacing.
             *
             */
            const ViewItemAdapterSList contains(const NmVector3 &point, const NmVector3 &spacing = NmVector3{1,1,1}) const;

            /** \brief Returns the list of segmentations whose bounds intersects the given bounds.
             * \param[in] bounds Bounds object.
             * \param[in] spacing View's spacing.
             *
             */
            const ViewItemAdapterSList intersects(const Bounds &bounds, const NmVector3 &spacing = NmVector3{1,1,1}) const;

          public slots:
            /** \brief Forces the recomputation of the AABB of the node and its ancestors.
             *
             */
            void refit();

          private:
            /** \brief Recomputes the AABB without iterating the elements.
             * \param propagate True to propagate the refit to parent and false otherwise.
             *
             */
            void refit(bool propagate);

            /** \brief Returns the best axis for splitting for the current AABB
             *
             */
            const Axis bestSplitAxis() const;

            /** From "Fast, Effective BVH Updates for Animated Scenes". None means no rotation, the next four are
             *  child to grandchild rotations and the last two are grandchild to grandchild rotations.
             */
            enum class Rotation: int8_t { NONE = 0, L_RL, L_RR, R_LL, R_LR, LL_RR, LL_RL };

            /** \brief Evaluates the given rotation and returns the evaluated cost.
             * \param[in] rot Rotation scheme.
             *
             * NOTE: The SA heuristics comes from the paper.
             *
             */
            const double evaluate(const Rotation rot);

            /** \brief Performs the given rotation on the node.
             * \param[in] rot Rotation scheme.
             *
             * NOTE: Step 1: swap node locations
             *       Step 2: update the node depth if child to grandchild rotation.
             *       Step 3: update the parent pointers.
             *       Step 4: Recompute the AABB for all.
             *
             */
            void rotate(const Rotation rot);

            DBVHNode                 *m_parent;  /** parent node of this one or nullptr if root.  */
            std::shared_ptr<DBVHNode> m_left;    /** left part of the tree or nullptr if leaf.    */
            std::shared_ptr<DBVHNode> m_right;   /** right part of the tree or nullptr if leaf.   */
            ViewItemAdapterSPtr       m_element; /** stored element if leaf, nullptr otherwise.   */
            unsigned int              m_depth;   /** depth of this node.                          */
            int                       m_size;    /** number of subnodes in the tree of this node. */
            Bounds                    m_bounds;  /** bounding box of all the contained elements.  */
        };
      } // namespace Utils
    } //namespace Model
  } // namespace GUI
} // namespace ESPINA


#endif // GUI_MODEL_UTILS_DBVH_H_
