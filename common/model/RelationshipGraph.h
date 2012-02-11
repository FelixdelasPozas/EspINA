/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  Jorge Peña Pastor <jpena@cesvima.upm.es>

    This program is free software: you can redistribute it and/or modify
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
#ifndef RELATIONSHIPGRAPH_H
#define RELATIONSHIPGRAPH_H

#include <boost/graph/adjacency_list.hpp>

#include <QMap>
#include <QSharedPointer>
#include <QString>
#include <QTextStream>

//Forward declarations
class ModelItem;

/// Graph like structure which contains all the relationships
/// between different elements of the model
class RelationshipGraph
{
  typedef unsigned int IndexType;

  struct VertexProperty
  {
    /// A pointer to the object associated with this vertex
    ModelItem *item;//TODO: Use smart pointers?
    /// Following members are needed to make the graph persistent
    std::string name;
    std::string shape;
    std::string args;
  };
  typedef boost::property
  < boost::vertex_index1_t
  , IndexType
  , VertexProperty
  > VertexProperties;

  struct EdgeProperty
  {
    /// Relationships between model items connected by an edge
    std::string relationship;
  };
  typedef boost::property
  < boost::edge_name_t
  , std::string
  > EdgeProperties;

  struct GraphPropery
  {
    std::string owner;
  };

  typedef GraphPropery GraphProperties;

//   /// SubGraphs REQUIRE vertex and edge properties
  typedef boost::adjacency_list
  < boost::listS
  , boost::vecS
  , boost::directedS
  , VertexProperties
  , EdgeProperties
  , GraphProperties
  > Graph;

  typedef Graph::vertex_descriptor VertexId;

  typedef Graph::edge_descriptor EdgeId;

  typedef boost::graph_traits<Graph> GraphTraits;

  typedef GraphTraits::vertex_descriptor VertexDescriptor;

  typedef GraphTraits::vertex_iterator VertexIterator;

  typedef GraphTraits::out_edge_iterator OutEdgeIterator;

  typedef std::pair<OutEdgeIterator, OutEdgeIterator> EdgeIteratorRange;

public:
  enum PrintFormat
  { GRAPHVIZ
  , DEBUG
  };

  RelationshipGraph();
  ~RelationshipGraph(){}

  void addItem(ModelItem    *item);
  void removeItem(ModelItem *item);

  void addRelation(ModelItem* ancestor, ModelItem* successor, const QString description);
  void connect(const QString& ancestor, ModelItem* successor, const QString description);

//   void readTrace(std::istream& content);
  void readTrace(QTextStream& stream);

//   void registerPlugin(QString key, IFilterFactory* factory);
//   IFilterFactory* getRegistredPlugin(QString& key);
  /*
  void addSubtrace(const ProcessingTrace *subTrace);
  std::vector<ITraceNode *> inputs(const ITraceNode *node);
  std::vector<ITraceNode *> outputs(const ITraceNode *node);
  */
  void print(std::ostream& out, PrintFormat format = GRAPHVIZ);

private:
  /// Update vertex's information with model's items' information
  void updateVertexInformation();
  //! It retrieves the current vertex index of a ModelItem
  VertexDescriptor vertex(ModelItem* item);
  //!Convert a string int the correct format "{argument:value;}+" in a NodeParamList
  //ITraceNode::Arguments parseArgs( QString& raw );

  // attributes
  Graph m_graph;
};

typedef QSharedPointer<RelationshipGraph> RelationshipGraphPtr;

#endif // RELATIONSHIPGRAPH_H
