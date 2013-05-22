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
  /// Graph like structure which contains all the relationships
  /// between different elements of the model
  class RelationshipGraph
  {
  public:
    struct Vertex
    {
      Vertex()
      : item(NULL) {}

      /// A pointer to the object associated with this vertex
      ModelItemPtr item;
      /// Following members are needed to make the graph persistent
      std::string  name;
      std::string  shape;
      std::string  args;
      size_t       descriptor;
    };

    struct EdgeProperty
    {
      /// Relationships between model items connected by an edge
      std::string relationship;
    };

    struct Edge
    {
      Vertex source;
      Vertex target;
      std::string    relationship;
    };

    typedef QList<Vertex> Vertices;
    typedef QList<Edge> Edges;

  private:
    typedef unsigned int IndexType;

    typedef boost::adjacency_list
    < boost::listS
    , boost::vecS
    , boost::bidirectionalS
    , Vertex
    , EdgeProperty
    > Graph;

    typedef boost::graph_traits<Graph>     GraphTraits;

  public:
    typedef GraphTraits::edge_descriptor   EdgeDescriptor;
    typedef GraphTraits::vertex_descriptor VertexDescriptor;
    typedef GraphTraits::vertex_iterator   VertexIterator;

    typedef GraphTraits::edge_iterator     EdgeIterator;
    typedef GraphTraits::in_edge_iterator  InEdgeIterator;
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

    /// Remove all vertices and edges from the graph
    void clear() {m_graph.clear();}

    void addItem   (ModelItemPtr item);
    void removeItem(ModelItemPtr item);

    /// Add given relation if realtion doesn't already existsI
    void addRelation   (ModelItemPtr  ancestor,
                        ModelItemPtr  successor,
                        const QString &description);

    /// Remove given relation if it exists
    void removeRelation(ModelItemPtr  ancestor,
                        ModelItemPtr  successor,
                        const QString &description);

    /// Return all graph's edges
    Edges edges(const QString &filter = "");

    /// Return a list of edges whose destination vertex is v
    Edges inEdges (Vertex v, const QString &filter = "");

    /// Return a list of edges whose source vertex is v
    Edges outEdges(Vertex v, const QString &filter = "");

    /// Return a list of edges whose source or destination vertex is v
    Edges edges   (Vertex v, const QString &filter = "");

    /// Remove all edges whose source or destination vertex is v
    void removeEdges(Vertex v);

    /// Return all graph's vertices 
    Vertices vertices() const;

    /// Return all vertices whose outgoing edges end on v
    Vertices ancestors(Vertex v, const QString &filter = "") const;

    /// Return all vertices whose incoming edges start on v
    Vertices succesors(Vertex v, const QString &filter = "") const;

    void setItem(Vertex v, ModelItemPtr item);

    static ModelItemType type(const Vertex v);

    void read (std::istream& stream, RelationshipGraph::PrintFormat format = BOOST);

    void write(std::ostream& stream, RelationshipGraph::PrintFormat format = BOOST);

    //   void print(std::ostream& out, PrintFormat format = GRAPHVIZ);
    /// Update vertex's information with model's items' information
    void updateVertexInformation();

    //! Retrieve current vertex index of a ModelItem
    Vertex vertex(ModelItemPtr item) const;
    VertexDescriptor vertex(VertexDescriptor v);

  private:
    bool findRelation(const VertexDescriptor source,
                      const VertexDescriptor destination,
                      const QString         &relation,
                      OutEdgeIterator       &edge
                     ) const;

  private:
    mutable Graph m_graph;
  };

  typedef boost::shared_ptr<RelationshipGraph> RelationshipGraphPtr;
}

#endif // RELATIONSHIPGRAPH_H
