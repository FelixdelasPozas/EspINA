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

#include <Core/EspinaCore_Export.h>

// ESPINA
#include <Core/Analysis/Segmentation.h>

// VTK
#include <vtkSmartPointer.h>
#include <vtkMath.h>

// Qt
#include <QList>
#include <QString>
#include <QDebug>

// C++
#include <cmath>

class vtkPolyData;

namespace ESPINA
{
  namespace Core
  {
    /** \struct SkeletonNode
     * \brief Definition of a point in 3D coordinates and it's connections and annotations.
     *
     */
    struct EspinaCore_EXPORT SkeletonNode
    {
      double                position[3]; /** position of the node in space.        */
      QList<SkeletonNode *> connections; /** list of connections with other nodes. */
      QString               id;          /** identifier.                           */

      /** \brief SkeletonNode struct constructor with coordinates.
       * \param[in] worldPos pointer to values.
       *
       */
      SkeletonNode(const double worldPos[3])
      {
        ::memcpy(position, worldPos, 3*sizeof(double));
      }

      /** \brief SkeletonNode struct constructor with initializer list.
       * \param[in] list list of values.
       */
      SkeletonNode(std::initializer_list<double> list)
      {
        Q_ASSERT(list.size() == 3);
        std::copy(list.begin(), list.end(), position);
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
    using Conenction    = QPair<SkeletonNode *, Segmentation *>;
    using Connections   = QList<Conenction>;

    struct EspinaCore_EXPORT Path
    {
      SkeletonNode *begin;
      SkeletonNode *end;
      SkeletonNodes seen;
      QString       note;

      Path(): begin{nullptr}, end{nullptr} {};

      double length()
      {
        double total = 0;
        if(seen.size() == 1) return 0;
        for(int i = 0; i < seen.size() - 1; ++i)
        {
          auto partial = vtkMath::Distance2BetweenPoints(seen.at(i)->position, seen.at(i+1)->position);
          total += std::sqrt(partial);
        }

        return total;
      }
    };

    using PathList      = QList<Path>;

    /** \brief Operator == for skeleton nodes.
     * \param[in] left path object.
     * \param[in] right path object.
     *
     */
    bool EspinaCore_EXPORT operator==(const SkeletonNode &left, const SkeletonNode &right);

    /** \brief Operator == for paths.
     * \param[in] left path object.
     * \param[in] right path object.
     *
     */
    bool EspinaCore_EXPORT operator==(const Path &left, const Path &right);

    /** \brief Operator << for QDebug and a skeleton in nodes list form.
     * \param[in] stream QDebug stream
     * \param[in] skeleton skeleton in nodes list form.
     *
     */
    QDebug EspinaCore_EXPORT operator<<(QDebug stream, const SkeletonNodes &skeleton);

    /** \brief Operator << for QDebug and Path struct.
     * \param[in] stream QDebug stream
     * \param[in] path path struct.
     *
     */
    QDebug EspinaCore_EXPORT operator<<(QDebug stream, const struct Path &path);

    /** \brief Converts a vtk structure to Skeleton nodes list.
     * \param[in] skeleton skeleton polydata object.
     *
     */
    SkeletonNodes EspinaCore_EXPORT toNodes(const vtkSmartPointer<vtkPolyData> skeleton);

    /** \brief Converts a list of nodes to a vtk mesh object.
     * \param[in] nodes list of nodes.
     *
     */
    vtkSmartPointer<vtkPolyData> EspinaCore_EXPORT toPolyData(const SkeletonNodes &nodes);

    /** \brief Returns the index of the closest node to the given position in space.
     * \param[in] point coordinates of a point in space.
     * \param[in] nodes list of nodes.
     *
     */
    int EspinaCore_EXPORT closestNode(const double position[3], const SkeletonNodes nodes);

    /** \brief Returns the distance closest point in the skeleton to the given point coordinates.
     * \param[in] point input point coordinates.
     * \param[in] nodes skeleton nodes list.
     * \param[out] node_i node of the line containing the closest point to the input point.
     * \param[out] node_j node of the line containing the closest point to the input point.
     * \param[out] worldPosition position of the closest point in the skeleton to the input point.
     *
     */
    double EspinaCore_EXPORT closestDistanceAndNode(const double position[3], const SkeletonNodes nodes, int &node_i, int &node_j, double worldPosition[3]);

    /** \brief Returns all the paths in the skeleton as a list of connected relevant nodes.
     * \param[in] skeleton skeleton in nodes list form.
     *
     * NOTE: this method doesn't handle unconnected graphs, so it's better to call connected components method first and then this on every component.
     *
     */
    PathList EspinaCore_EXPORT paths(const SkeletonNodes &skeleton);

    /** \brief Returns the loops in the given skeleton.
     * \param[in] skeleton skeleton structure as a list of nodes.
     *
     * NOTE: this method doesn't handle unconnected graphs, so it's better to call connected components method first and then this on every component.
     *
     */
    QList<SkeletonNodes> EspinaCore_EXPORT loops(const SkeletonNodes &skeleton);

    /** \brief Returns the skeleton as a list of connected components
     * \param[in] skeleton skeleton as as list of nodes.
     *
     */
    QList<SkeletonNodes> EspinaCore_EXPORT connectedComponents(const SkeletonNodes &skeleton);

    /** \brief Annotates relevant nodes in the given structure.
     * \param[in] nodes nodes list.
     */
    void EspinaCore_EXPORT annotateNodes(SkeletonNodes nodes);

  } // namespace Core
} // namespace ESPINA

#endif // CORE_ANALYSIS_DATA_SKELETONDATAUTILS_H_
