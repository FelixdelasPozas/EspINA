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

#ifndef CORE_ANALYSIS_DATA_SKELETONDATAUTILS_H_
#define CORE_ANALYSIS_DATA_SKELETONDATAUTILS_H_

// VTK
#include <vtkSmartPointer.h>

// Qt
#include <QList>
#include <QString>

class vtkPolyData;

namespace ESPINA
{
  namespace Core
  {
    /** \struct SkeletonNode
     * \brief Definition of a point in 3D coordinates and it's connections and annotations.
     *
     */
    struct SkeletonNode
    {
      double                position[3]; /** position of the node in space.        */
      QList<SkeletonNode *> connections; /** list of connections with other nodes. */
      QString               id;          /** identifier.                           */

      /** \brief SkeletonNode struct constructor with coordinates.
       *
       */
      SkeletonNode(const double worldPos[3])
      {
        position[0] = worldPos[0];
        position[1] = worldPos[1];
        position[2] = worldPos[2];
      }

      /** \brief SkeletonNode struct constructor with coordinates and identifier.
       *
       */
      SkeletonNode(double worldPos[3], const QString identifier)
      : SkeletonNode{worldPos}
      {
        id = identifier;
      }

      /** \brief SkeletonNode struct destructor.
       *
       */
      ~SkeletonNode()
      {
        for(auto node: connections) node->connections.removeAll(this);
      }
    };

    using SkeletonNodes = QList<SkeletonNode *>;

    /** \brief Converts a vtk structure to Skeleton nodes list.
     * \param[in] skeleton skeleton polydata object.
     *
     */
    SkeletonNodes toNodes(const vtkSmartPointer<vtkPolyData> skeleton);

    /** \brief Converts a list of nodes to a vtk mesh object.
     * \param[in] nodes list of nodes.
     *
     */
    vtkSmartPointer<vtkPolyData> toPolyData(const SkeletonNodes nodes);

    /** \brief Returns the index of the closest node to the given position in space.
     * \param[in] point coordinates of a point in space.
     * \param[in] nodes list of nodes.
     *
     */
    int closestNode(const double position[3], const SkeletonNodes nodes);

    /** \brief Returns the distance closest point in the skeleton to the given point coordinates.
     * \param[in] point input point coordinates.
     * \param[in] nodes skeleton nodes list.
     * \param[out] node_i node of the line containing the closest point to the input point.
     * \param[out] node_j node of the line containing the closest point to the input point.
     * \param[out] worldPosition position of the closest point in the skeleton to the input point.
     *
     */
    double closestDistanceAndNode(const double position[3], const SkeletonNodes nodes, int &node_i, int &node_j, double worldPosition[3]);
  } // namespace Core
} // namespace ESPINA

#endif // CORE_ANALYSIS_DATA_SKELETONDATAUTILS_H_
