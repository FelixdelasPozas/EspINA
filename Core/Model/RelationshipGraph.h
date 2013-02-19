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

#include "Core/EspinaTypes.h"

#include <QMap>
#include <QString>
#include <QTextStream>

#include <QDebug>

namespace EspINA
{
  struct VertexProperty
  {
    VertexProperty() {}//: item() {}
    /// A pointer to the object associated with this vertex
    ModelItemPtr item;
    /// Following members are needed to make the graph persistent
    std::string  name;
    std::string  shape;
    std::string  args;
    unsigned int vId;
  };

  struct Edge
  {
    VertexProperty source;
    VertexProperty target;
    std::string    relationship;
  };

  typedef QList<VertexProperty> Vertices;
  typedef QList<Edge> Edges;

  class   RelationshipGraph;
  typedef QSharedPointer<RelationshipGraph> RelationshipGraphPtr;

  /// Graph like structure which contains all the relationships
  /// between different elements of the model
  class RelationshipGraph
  {
  public:
    typedef unsigned int IndexType;

    struct EdgeProperty
    {
      /// Relationships between model items connected by an edge
      std::string relationship;
    };

    struct GraphPropery
    {
      std::string owner;
    };

    typedef GraphPropery GraphProperties;

    //   /// SubGraphs REQUIRE vertex and edge properties
    typedef boost::adjacency_list
    < boost::listS
    , boost::vecS
    , boost::bidirectionalS
    , VertexProperty
    , EdgeProperty
    , GraphProperties
    > Graph;

    typedef Graph::vertex_descriptor VertexId;

    //   typedef QList<VertexId> Vertices;

    typedef Graph::edge_descriptor EdgeId;

    typedef boost::graph_traits<Graph> GraphTraits;

    typedef GraphTraits::vertex_descriptor VertexDescriptor;

    typedef GraphTraits::vertex_iterator VertexIterator;

    typedef GraphTraits::edge_iterator EdgeIterator;
    typedef GraphTraits::in_edge_iterator InEdgeIterator;
    typedef GraphTraits::out_edge_iterator OutEdgeIterator;

    typedef std::pair<OutEdgeIterator, OutEdgeIterator> EdgeIteratorRange;

  public:
    enum PrintFormat
    { BOOST
    , GRAPHVIZ
    , DEBUG
    };

    RelationshipGraph();
    ~RelationshipGraph(){}

    void clear() {m_graph.clear();}

    void addItem   (ModelItemPtr item);
    void removeItem(ModelItemPtr item);

    void addRelation   (ModelItemPtr  ancestor,
                        ModelItemPtr  successor,
                        const QString &description);
    void removeRelation(ModelItemPtr  ancestor,
                        ModelItemPtr  successor,
                        const QString &description);
    void connect(const QString &ancestor,
                 ModelItemPtr   successor,
                 const QString &description);

    //   Vertices rootVertices();
    Edges edges(const QString &filter = "");

    Edges inEdges (VertexId v, const QString &filter = "");
    Edges outEdges(VertexId v, const QString &filter = "");
    Edges edges   (VertexId v, const QString &filter = "");
    void removeEdges(VertexId v);

    Vertices vertices();
    Vertices ancestors(VertexId v, const QString &filter = "");
    Vertices succesors(VertexId v, const QString &filter = "");

    bool find(VertexProperty vp, VertexProperty &foundV);

    void setItem(VertexId v, ModelItemPtr item);
    QString name(VertexId v) const;

    static ModelItemType type(const VertexProperty v);

    QString args(VertexId v) const;
    VertexProperty properties(VertexId v);

    //   void readTrace(std::istream& content);
    void load (QTextStream& serialization);
    void read (std::istream& stream, RelationshipGraph::PrintFormat format = BOOST);
    void write(std::ostream& stream, RelationshipGraph::PrintFormat format = BOOST);

    /*
     *  void addSubtrace(const ProcessingTrace *subTrace);
     *  std::vector<ITraceNode *> inputs(const ITraceNode *node);
     *  std::vector<ITraceNode *> outputs(const ITraceNode *node);
     */
    //   void print(std::ostream& out, PrintFormat format = GRAPHVIZ);
    /// Update vertex's information with model's items' information
    void updateVertexInformation();

    //! Retrieve current vertex index of a ModelItem
    VertexDescriptor vertex(ModelItemPtr item);
    VertexDescriptor vertex(VertexId v);

  private:
    Graph m_graph;
  };
}

#endif // RELATIONSHIPGRAPH_H
