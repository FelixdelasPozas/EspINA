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
#include <cstring>

class vtkPolyData;

namespace ESPINA
{
  namespace Core
  {
    /** \struct SkeletonStroke
     * \brief Skeleton stroke definition struct.
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
        : name{QString()}, colorHue{-1}, type{0}, useMeasure{false}
        {};

        /** \brief Operator == for skeleton strokes.
         * \param[in] other other skeleton to compare with.
         */
        bool operator==(const SkeletonStroke &other) const
        {
          return (name       == other.name)     &&
                 (colorHue   == other.colorHue) &&
                 (type       == other.type)     &&
                 (useMeasure == other.useMeasure);
        }

        /** \brief Operator != for skeleton strokes.
         * \param[in] other other skeleton to compare with.
         *
         */
        bool operator!=(const SkeletonStroke &other) const
        {
          return !(*this == other);
        }

        /** \brief Operator < for skeleton strokes.
         * \param[in] other other skeleton to compare with.
         *
         */
        bool operator<(const SkeletonStroke &other) const
        {
           return (name < other.name);
        }

        /** \brief Operator = for skeleton strokes.
         * \param[in] other other skeleton definition.
         *
         */
        SkeletonStroke& operator=(const SkeletonStroke &other)
        {
          colorHue   = other.colorHue;
          name       = other.name;
          type       = other.type;
          useMeasure = other.useMeasure;

          return *this;
        }

        /** \brief Returns true if the stroke is not defined and false otherwise.
         *
         */
        bool isNull() const
        {
          return name.isEmpty();
        }
    };

    /** \brief Registers the skeleton data types operators in Qt metadata structures for signal/slots operations.
     *
     */
    void EspinaCore_EXPORT registerSkeletonDataOperators();

    using SkeletonStrokes = QList<struct SkeletonStroke>;

    /** \struct SkeletonEdge
     * \brief Defines the data of an edge in a SkeletonDefinition.
     *
     */
    struct EspinaCore_EXPORT SkeletonEdge
    {
      int strokeIndex;  /** index to stroke list.                    */
      int strokeNumber; /** number of the stroke.                    */
      int parentEdge;   /** number of the parent edge, -1 if orphan. */

      /** \brief SkeletonEdge struct constructor.
       *
       */
      explicit SkeletonEdge(): strokeIndex{-1}, strokeNumber{-1}, parentEdge{-1} {};

      /** \brief SkeletonEdge struct constructor.
       * \param[in] index Index in the stroke list.
       * \param[in] number number of stroke.
       * \param[in] parent Index of the parent stroke in the stroke list.
       *
       */
      explicit SkeletonEdge(int index, int number, int parent): strokeIndex{index}, strokeNumber{number}, parentEdge{parent} {};

      /** \brief Operator == for SkeletonEdge struct.
       * \param[in] other SkeletonEdge struct reference to compare.
       *
       */
      bool operator==(const SkeletonEdge &other) const
      { return std::memcmp(&other, this, sizeof(SkeletonEdge)) == 0; }

      /** \brief Operator < for SkeletonEdge struct.
       * \param[in] other SkeletonEdge struct reference to compare.
       *
       */
      bool operator<(const SkeletonEdge &other) const
      { return strokeIndex < other.strokeIndex; }

      /** \brief Returns true if the edge has no parent.
       *
       */
      bool isOrphan() const
      { return parentEdge == -1; }

      /** \brief Operator = for SkeletonEdge struct.
       * \param[in] other SkeletonEdge struct values to assign.
       *
       */
      SkeletonEdge &operator=(const SkeletonEdge &other)
      { std::memcpy(this, &other, sizeof(SkeletonEdge)); return *this; }
    };

    using SkeletonEdges = QList<struct SkeletonEdge>;

    /** \brief Properties for nodes.
     *
     */
    enum SkeletonNodeProperty: int
    {
      TRUNCATED = 1 << 0 /** the path that contains the node is truncated at the node position. */
    };

    Q_DECLARE_FLAGS(SkeletonNodeFlags, SkeletonNodeProperty)
    Q_DECLARE_OPERATORS_FOR_FLAGS(SkeletonNodeFlags)


    /** \struct SkeletonNode
     * \brief Definition of a point in 3D coordinates and it's connections and properties.
     *
     */
    struct EspinaCore_EXPORT SkeletonNode
    {
      double                    position[3];  /** position of the node in space.                                       */
      QMap<SkeletonNode *, int> connections;  /** maps the connections with other nodes to the index in the edge list. */
      SkeletonNodeFlags         flags;

      /** \brief SkeletonNode struct constructor with coordinates.
       * \param[in] worldPos pointer to values.
       *
       */
      explicit SkeletonNode(const double worldPos[3])
      {
        std::memcpy(position, worldPos, 3*sizeof(double));
      }

      /** \brief SkeletonNode struct constructor with initializer list.
       * \param[in] list list of values.
       */
      explicit SkeletonNode(std::initializer_list<double> list)
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

      /** \brief Returns true if the node is a stroke terminal node and false otherwise.
       *
       */
      bool isTerminal() const
      {
        const auto connectionNodes = this->connections.keys();
        return connectionNodes.size() == 1 && !connectionNodes.contains(const_cast<SkeletonNode *>(this));
      }

      /** \brief Returns true if the node is a branching node and false otherwise.
       *
       */
      bool isBranching() const
      {
        auto connectionsSet = this->connections.values().toSet(); // unique edges only
        return this->connections.size() > 2 && !this->connections.keys().contains(const_cast<SkeletonNode *>(this)) && connectionsSet.size() >= 2;
      }

      /** \brief Operator == for skeleton nodes.
       * \param[in] other other skeleton node for comparison.
       *
       */
      bool operator==(const Core::SkeletonNode &other) const;
    };

    using SkeletonNodes = QList<SkeletonNode *>;

    /** \struct Path
     * \brief Defines the structure of a path in a skeleton definition, listing its nodes and a begin and an ending node.
     *
     */
    struct EspinaCore_EXPORT Path
    {
      SkeletonNode *begin;   /** path begin node.                                                                   */
      SkeletonNode *end;     /** path end node.                                                                     */
      SkeletonNodes seen;    /** list of nodes of path path, including begin and end & ordered begin->end.          */
      QString       note;    /** annotation, normally the name of the stroke of the path.                           */
      int           edge;    /** index of the path edge of the path in the list of edges of SkeletonDefinition.     */
      int           stroke;  /** index of the path stroke of the path in the list of strokes of SkeletonDefinition. */

      /** \brief Path struct constructor.
       *
       */
      explicit Path(): begin{nullptr}, end{nullptr}, edge{-1}, stroke{-1} {};

      /** \brief Returns the length of the path.
       *
       */
      double length() const
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

      /** \brief Returns true if the path connects with the given one. Implies direction, so not reflexive.
       * \param[in] path Path object.
       *
       */
      bool connectsTo(const Path &path) const;

      /** \brief Returns true if the given point is an ending of the path.
       * \param[in] point Point coordinates
       *
       */
      bool hasEndingPoint(const NmVector3 &point) const;

      /** \brief Operator == for paths.
       * \param[in] other other path for comparison.
       *
       */
      bool operator==(const Path &other) const;

      /** \brief Operator == for paths.
       * \param[in] other other path for comparison.
       *
       */
      bool operator!=(const Path &other) const
      { return !(*this == other); }

      /** \brief Operator < for paths, rather arbitrary.
       * \param[in] other other path for comparison and ordering.
       *
       */
      bool operator<(const Path &other) const;
    };

    using PathList = QList<Path>;

    /** \struct SkeletonDefinition
     * \brief Contains all needed data for a skeleton based on strokes.
     *
     */
    struct EspinaCore_EXPORT SkeletonDefinition
    {
        SkeletonNodes             nodes;   /** nodes positions and connections.   */
        SkeletonEdges             edges;   /** edges definitions of the skeleton. */
        SkeletonStrokes           strokes; /** list of used strokes.              */
        QMap<SkeletonStroke, int> count;   /** maps strokes and times used.       */

        /** brief Clears the structure.
         *
         */
        void clear()
        {
          for(auto node: nodes) delete node;
          nodes.clear();
          edges.clear();
          count.clear();
          strokes.clear();
        }
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
     * NOTE: in the case that the closest distance is between nodes i and j, node_i is the closest one.
     *
     */
    double EspinaCore_EXPORT closestDistanceAndNode(const double position[3], const SkeletonNodes nodes, int &node_i, int &node_j, double worldPosition[3]);

    /** \brief Returns the distance and the closest point in the given segment to the input position in space.
     * \param[in] point input point coordinates.
     * \param[out] node_i begin node of the line segment.
     * \param[out] node_j end node of the line segment.
     * \param[out] worldPosition position of the closest point in the segment to the input point.
     *
     */
    double EspinaCore_EXPORT closestPointToSegment(const double position[3], const SkeletonNode *node_i, const SkeletonNode *node_j, double closestPoint[3]);

    /** \brief Returns all the paths in the skeleton as a list of connected relevant nodes.
     * \param[in] nodes list of connected nodes.
     * \param[in] edges list of defined edges.
     * \param[in] strokes list of defined strokes.
     *
     * NOTE: this method doesn't handle unconnected graphs, so it's better to call connected components method first and then this on every component.
     *
     */
    PathList EspinaCore_EXPORT paths(const SkeletonNodes &nodes, const SkeletonEdges &edges, const SkeletonStrokes &strokes);

    /** \brief Returns all the paths in the skeleton as a list of connected relevant nodes.
     * \param[in] skeleton Skeleton definition data.
     *
     */
    inline PathList paths(const SkeletonDefinition &skeleton)
    { return paths(skeleton.nodes, skeleton.edges, skeleton.strokes); };

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
     * \param[in] edges SkeletonEdge list.
     *
     */
    QString EspinaCore_EXPORT strokeName(const Core::SkeletonEdge &edge, const Core::SkeletonStrokes &strokes, const Core::SkeletonEdges &edges);

    /** \brief Adjusts the node stroke numbers to miminize stroke count.
     * \param[in] skeleleton Skeleton definition reference.
     */
    void EspinaCore_EXPORT adjustStrokeNumbers(Core::SkeletonDefinition &skeleton);

    /** \brief Computes and returns the angle between the edges base-a and base-b.
     * \param[in] base common brach node.
     * \param[in] a first node of branch A.
     * \param[in] b first node of branch B.
     *
     */
    double EspinaCore_EXPORT angle(SkeletonNode *base, SkeletonNode *a, SkeletonNode *b);

    /** \struct PathHierarchyNode
     * \brief Defines a node of a hierarchy of paths. Where the children paths have a node in the node path.
     */
    struct EspinaCore_EXPORT PathHierarchyNode
    {
        Path                       path;     /** path of this node.                */
        PathHierarchyNode         *parent;   /** parent of the path of the node.   */
        QList<PathHierarchyNode *> children; /** children of the path of the node. */

        /** \brief PathHierarchyNode struct constructor.
         * \param[in] path Path struct.
         *
         */
        explicit PathHierarchyNode(const Path &path) : path{path}, parent{nullptr} {};

        /** \brief PathHierarchyNode struct destructor.
         *
         */
        ~PathHierarchyNode() { for(auto child: children) delete child; };
    };

    /** \brief Returns the path hierarchy of the given skeleton. The root node has an empty path and null parent. If the edges
     * or strokes is empty then there won't be applied any stroke priority.
     * \param[in] paths Skeleton path list.
     * \param[in] edges Skeleton edges list.
     * \param[in] strokes Skeleton strokes list.
     *
     */
    QList<PathHierarchyNode *> EspinaCore_EXPORT pathHierarchy(const PathList              &paths,
                                                               const Core::SkeletonEdges   &edges   = Core::SkeletonEdges(),
                                                               const Core::SkeletonStrokes &strokes = Core::SkeletonStrokes());

    /** \brief Returns the node containing the given path in the given hierarchy or nullptr if not found in the hierarchy of nodes.
     * \param[in] path Path to search.
     * \param[in] hierarchy List of nodes.
     *
     */
    PathHierarchyNode * EspinaCore_EXPORT locatePathHierarchyNode(const Path &path, const QList<PathHierarchyNode *> &hierarchy);

    /** \brief Returns true if the node or any of its children is truncated.
     *
     */
    bool EspinaCore_EXPORT isTruncated(const PathHierarchyNode *node);

    /** \brief Returns the length of the node and its subnodes if the paths are 'Spine' or 'Subspine'.
     * \param[in] node Hierarchy node.
     *
     */
    double EspinaCore_EXPORT length(const PathHierarchyNode *node);

    /** \brief Returns the points of connections in the given node and its children from the connectionPoints list.
     * \param[in] node Hierarchy node.
     * \param[in] connectionsPoints List of connection points.
     *
     */
    QList<NmVector3> EspinaCore_EXPORT connectionsInNode(const PathHierarchyNode *node, const QList<NmVector3> &connectionPoints);

    /** \brief Helper method that removes unused strokes.
     * \param[in] skeleton Skeleton definition struct.
     *
     */
    void EspinaCore_EXPORT cleanSkeletonStrokes(SkeletonDefinition &skeleton);

    /** \brief Helper method that removes nodes without connections from the list of given nodes.
     * \param[in] nodes List of nodes.
     *
     */
    void EspinaCore_EXPORT removeIsolatedNodes(SkeletonNodes &nodes);

    /** \brief Helper method that merges nodes with the same position.
     * \param[in] nodes List of nodes.
     *
     */
    void EspinaCore_EXPORT mergeSamePositionNodes(SkeletonNodes &nodes);

    /** \brief Returns the alternate hue of the given stroke index if it matches with the hue of other strokes.
     * \param[in] strokes Stroke list.
     * \param[in] index Index of the stroke to compute hue.
	 * \param[in] cateogoryHue Hue value of segmentation's category color. 
     *
     */
    QColor EspinaCore_EXPORT alternateStrokeColor(const SkeletonStrokes &strokes, int index, int categoryHue);

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
