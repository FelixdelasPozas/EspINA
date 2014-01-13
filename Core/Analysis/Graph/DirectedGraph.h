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
#ifndef ESPINA_DIRECTED_GRAPH_H
#define ESPINA_DIRECTED_GRAPH_H

#include "EspinaCore_Export.h"

#include <boost/graph/adjacency_list.hpp>

#include "Core/EspinaTypes.h"
#include "Core/IO/GraphIO.h"

#include <QTextStream>

#include <QDebug>

namespace EspINA
{
  /// Graph like structure which contains all the relationships
  /// between different elements of the model
  class EspinaCore_EXPORT DirectedGraph
  {
  public:
    struct Null_Item_Exception {};
    struct Existing_Item_Exception {};
    struct Existing_Relation_Exception {};
    struct Item_Not_Found_Exception {};
    struct Relation_Not_Found_Exception {};

  public:
    using Vertex    = PersistentSPtr;
    using VertexPtr = PersistentPtr;

    struct EdgeProperty
    {
      std::string relationship;
    };

    /** Relationships between analysis items connected by an edge
     *
     */
    struct Edge
    {
      Vertex      source;
      Vertex      target;
      std::string relationship;
    };

    typedef QList<Vertex> Vertices;
    typedef QList<Edge>   Edges;

  private:
    typedef unsigned int IndexType;
    // Boost edge implementation

    typedef boost::adjacency_list
    < boost::listS
    , boost::vecS
    , boost::bidirectionalS
    , Vertex
    , EdgeProperty
    > Graph;

    typedef boost::graph_traits<Graph>     GraphTraits;

    using EdgeDescriptor   = GraphTraits::edge_descriptor;
    using VertexDescriptor = GraphTraits::vertex_descriptor;
    using VertexIterator   = GraphTraits::vertex_iterator;

    using EdgeIterator     = GraphTraits::edge_iterator;
    using InEdgeIterator   = GraphTraits::in_edge_iterator;
    using OutEdgeIterator  = GraphTraits::out_edge_iterator;

    using EdgeIteratorRange = std::pair<OutEdgeIterator, OutEdgeIterator>;

  public:
    explicit DirectedGraph();
    ~DirectedGraph(){}

    /// Remove all vertices and edges from the graph
    void clear() {m_graph.clear();}

    void add   (Vertex vertex);
    void remove(Vertex vertex);

    /// Add given relation if realtion doesn't already existsI
    void addRelation(Vertex ancestor,
                     Vertex successor,
                     const QString &description);

    /// Remove given relation if it exists
    void removeRelation(Vertex  ancestor,
                        Vertex  successor,
                        const QString &description);

    bool contains(Vertex vertex);

    /// Return all graph's edges
    Edges edges(const QString &filter = "");

    /// Return a list of edges whose destination vertex is v
    Edges inEdges(Vertex vertex, const QString &filter = "");

    /// Return a list of edges whose source vertex is v
    Edges outEdges(Vertex vertex, const QString &filter = "");

    /// Return a list of edges whose source or destination vertex is v
    Edges edges   (Vertex vertex, const QString &filter = "");

    /// Remove all edges whose source or destination vertex is v
    void removeEdges(Vertex vertex);

    /// Return all graph's vertices
    Vertices vertices() const;

    /// Return all vertices whose outgoing edges end on v
    Vertices ancestors(Vertex vertex, const QString &filter = "") const;
    Vertices ancestors(VertexPtr vertex, const QString &filter = "") const;

    /// Return all vertices whose incoming edges start on v
    Vertices succesors(Vertex vertex, const QString &filter = "") const;
    Vertices succesors(VertexPtr vertex, const QString &filter = "") const;

  private:
    DirectedGraph::Vertex vertex(VertexDescriptor descriptor) const;
    DirectedGraph::VertexDescriptor descriptor(Vertex vertex) const;
    DirectedGraph::VertexDescriptor descriptor(VertexPtr vertex) const;

    DirectedGraph::OutEdgeIterator findRelation(const VertexDescriptor source,
                                                const VertexDescriptor destination,
                                                const QString         &relation) const;

  private:
    mutable Graph m_graph;

   friend void IO::Graph::read(std::istream& stream, DirectedGraphSPtr graph, IO::Graph::PrintFormat format);
   friend void IO::Graph::write(const DirectedGraphSPtr graph, std::ostream& stream, IO::Graph::PrintFormat format);
  };

  using DirectedGraphSPtr = std::shared_ptr<DirectedGraph>;
}

#endif // ESPINA_DIRECTED_GRAPH_H
