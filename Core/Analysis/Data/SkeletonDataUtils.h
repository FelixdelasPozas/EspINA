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

// C++
#include <cmath>

class vtkPolyData;

namespace ESPINA
{
  namespace Core
  {
    /** \struct SkeletonStroke
     * \brief Skeleton strokes definition struct.
     *
     */
    struct EspinaCore_EXPORT SkeletonStroke
    {
        QString name;       /** name of the stroke.                                       */
        int     colorHue;   /** color of the stroke.                                      */
        int     type;       /** type 0 solid, type 1 dashed.                              */
        bool    useMeasure; /** true to use the measure in the skeleton, false otherwise. */

        /** \brief SkeletonStroke struct constructor.
         * \param[in] strokeName  Stroke name.
         * \param[in] strokeColor Stroke color hue value.
         * \param[in] toUse       true to use the measure in the skeleton or false otherwise.
         *
         */
        explicit SkeletonStroke(QString strokeName, int strokeColor, int strokeType, bool toUse)
        : name{strokeName}, colorHue{strokeColor}, type{strokeType}, useMeasure{toUse}
        {};

        /** \brief SkeletonStroke struct empty constructor. Needed for QSettings loading.
         *
         */
        explicit SkeletonStroke()
        : name{QString()}, colorHue{0}, type{0}, useMeasure{false}
        {};

        /** \brief Operator == for skeleton strokes.
         * \param[in] other other skeleton to compare with.
         */
        bool operator==(const SkeletonStroke &other) const
        {
          return name == other.name && colorHue == other.colorHue && type == other.type && useMeasure == other.useMeasure;
        }

        /** \brief Operator != for skeleton strokes.
         * \param[in] other other skeleton to compare with.
         *
         */
        bool operator!=(const SkeletonStroke &other) const
        {
          return !(*this == other);
        }

        /** \brief Operator = for skeleton strokes.
         * \param[in] other other skeleton definition.
         *
         */
        void operator=(const SkeletonStroke &other)
        {
          colorHue   = other.colorHue;
          name       = other.name;
          type       = other.type;
          useMeasure = other.useMeasure;
        }

        /** \brief Operator < for skeleton strokes. Needed for using it in a QMap.
         *
         */
        bool operator<(const SkeletonStroke &other)
        {
          return (name < other.name);
        }

        /** \brief Returns true if the stroke is not defined and false otherwise.
         *
         */
        bool isNull() const
        {
          return name.isEmpty();
        }
    };

    /** \brief Registers the skeleton data types operators.
     *
     */
    void EspinaCore_EXPORT registerSkeletonDataOperators();

    using SkeletonStrokes = QList<struct SkeletonStroke>;

    /** \brief Struct SkeletonEdge
     *
     */
    struct SkeletonEdge
    {
      int strokeIndex;  /** index to stroke list. */
      int strokeNumber; /** number of the stroke. */

      /** \brief SkeletonEdge struct constructor.
       *
       */
      SkeletonEdge(): strokeIndex{-1}, strokeNumber{-1} {};

      /** \brief SkeletonEdge struct constructor.
       * \param[in] index Indes in the stroke list.
       * \param[in] number number of stroke.
       *
       */
      SkeletonEdge(int index, int number): strokeIndex{index}, strokeNumber{number} {};

      /** \brief Operator == for SkeletonEdge struct.
       * \param[in] other SkeletonEdge struct reference to compare.
       *
       */
      bool operator==(const SkeletonEdge &other)
      { return (strokeIndex == other.strokeIndex) && (strokeNumber == other.strokeNumber); }

      /** \brief Operator < for SkeletonEdge struct.
       * \param[in] other SkeletonEdge struct reference to compare.
       *
       */
      bool operator<(const SkeletonEdge &other)
      { return strokeIndex < other.strokeIndex; }
    };

    using SkeletonEdges = QList<struct SkeletonEdge>;

    /** \struct SkeletonNode
     * \brief Definition of a point in 3D coordinates and it's connections and annotations.
     *
     */
    struct EspinaCore_EXPORT SkeletonNode
    {
      double                    position[3];  /** position of the node in space.                                       */
      QMap<SkeletonNode *, int> connections;  /** maps the connections with other nodes to the index in the edge list. */

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

      /** \brief SkeletonNode struct destructor.
       *
       */
      ~SkeletonNode()
      {
        for(auto node: connections.keys())
        {
          node->connections.remove(this);
        }
      }

      /** \brief Operator == for skeleton nodes.
       * \param[in] other other skeleton node for comparison.
       *
       */
      bool operator==(const Core::SkeletonNode &other) const;
    };

    using SkeletonNodes = QList<SkeletonNode *>;

    struct EspinaCore_EXPORT Path
    {
      SkeletonNode *begin;
      SkeletonNode *end;
      SkeletonNodes seen;
      QString       note;

      /** \brief Path struct constructor.
       *
       */
      Path(): begin{nullptr}, end{nullptr} {};

      /** \brief Returns the length of the path.
       *
       */
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

      /** \brief Operator == for paths.
       * \param[in] other other path for comparison.
       *
       */
      bool operator==(const Path &other) const;
    };

    using PathList = QList<Path>;

    /** \struct SkeletonDefinition
     * \brief Contatins all needed data for a skeleton based on strokes.
     *
     */
    struct SkeletonDefinition
    {
        SkeletonNodes             nodes;   /** nodes positions and connections.   */
        SkeletonEdges             edges;   /** edges definitions of the skeleton. */
        SkeletonStrokes           strokes; /** list of used strokes.              */
        QMap<SkeletonStroke, int> count;   /** maps strokes and times used.       */
    };

    /** \brief Operator << for QDebug and a skeleton in nodes list form.
     * \param[in] stream QDebug stream
     * \param[in] skeleton skeleton definition struct.
     *
     */
    QDebug EspinaCore_EXPORT operator<<(QDebug stream, const SkeletonDefinition &skeleton);

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
    SkeletonDefinition EspinaCore_EXPORT toSkeletonDefinition(const vtkSmartPointer<vtkPolyData> skeleton);

    /** \brief Converts a list of nodes to a vtk mesh object.
     * \param[in] nodes list of nodes.
     *
     */
    vtkSmartPointer<vtkPolyData> EspinaCore_EXPORT toPolyData(const SkeletonDefinition &skeleton);

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
     * \param[in] nodes list of connected nodes.
     * \param[in] edges list of defined edges.
     * \param[in] strokes list of defined strokes.
     *
     * NOTE: this method doesn't handle unconnected graphs, so it's better to call connected components method first and then this on every component.
     *
     */
    PathList EspinaCore_EXPORT paths(const SkeletonNodes& nodes, const SkeletonEdges &edges, const SkeletonStrokes &strokes);

    /** \brief Returns the loops in the given skeleton.
     * \param[in] skeleton skeleton nodes list.
     *
     * NOTE: this method doesn't handle unconnected graphs, so it's better to call connected components method first and then this on every component.
     *
     */
    QList<SkeletonNodes> EspinaCore_EXPORT loops(const SkeletonNodes &skeleton);

    /** \brief Returns the skeleton as a list of connected components
     * \param[in] skeleton skeleton nodes list.
     *
     */
    QList<SkeletonNodes> EspinaCore_EXPORT connectedComponents(const SkeletonNodes &skeleton);

    /** \brief Returns the full name of the stroke.
     * \param[in] edge Edge of the stroke..
     * \param[in] strokes SkeletonStroke list.
     *
     */
    const QString EspinaCore_EXPORT strokeName(const Core::SkeletonEdge &edge, const Core::SkeletonStrokes &strokes);

    /** \brief Adjusts the node stroke numbers to miminize stroke count.
     * \param[in] skeleleton Skeleton definition reference.
     */
    void EspinaCore_EXPORT adjustStrokeNumbers(Core::SkeletonDefinition &skeleton);

  } // namespace Core
} // namespace ESPINA

Q_DECLARE_METATYPE(ESPINA::Core::SkeletonStroke);

/** \brief struct SkeletonStroke operator<< for serialization.
 * \param[in] out QDataStream output stream.
 * \parma[in] stroke SkeletonStroke struct.
 *
 */
QDataStream& operator<<(QDataStream& out, const ESPINA::Core::SkeletonStroke &stroke);

/** \brief struct SkeletonStroke operator>> for serialization.
 * \param[in] in QDataStream input stream.
 * \parma[in] stroke SkeletonStroke struct.
 *
 */
QDataStream& operator>>(QDataStream& in, ESPINA::Core::SkeletonStroke &stroke);

#endif // CORE_ANALYSIS_DATA_SKELETONDATAUTILS_H_