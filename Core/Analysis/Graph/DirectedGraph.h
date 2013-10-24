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

#include <QTextStream>

#include <QDebug>

namespace EspINA
{
  class DirectedGraph;
  using DirectedGraphSPtr = std::shared_ptr<DirectedGraph>;

  namespace IO {
    enum class PrintFormat
    { BOOST
    , DEBUG
    };

    void read (std::istream& stream, DirectedGraphSPtr graph, PrintFormat format = PrintFormat::BOOST);

    void write(const DirectedGraphSPtr graph, std::ostream& stream, PrintFormat format = PrintFormat::BOOST);
  }

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
    // Bundled Properties
    struct Vertex
    {
      Vertex()
      : item(PersistentSPtr())
      , descriptor(sizeof(int))
      {}

      // A pointer to the object associated with this vertex
      PersistentSPtr item;
//       // Following members are needed to make the graph persistent
//       std::string name;
//       std::string type;
//       std::string state;
      size_t      descriptor;
    };

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

    void addItem   (PersistentSPtr item);
    void removeItem(PersistentSPtr item);

    /// Add given relation if realtion doesn't already existsI
    void addRelation   (PersistentSPtr  ancestor,
                        PersistentSPtr  successor,
                        const QString &description);

    /// Remove given relation if it exists
    void removeRelation(PersistentSPtr  ancestor,
                        PersistentSPtr  successor,
                        const QString &description);

    bool contains(PersistentSPtr item);

    /// Retrieve current vertex index of a Persistent
    /// A vertex with NULL item field is returned if no vertex contains item
    Vertex vertex(PersistentSPtr item) const;

    /// Return all graph's edges
    Edges edges(const QString &filter = "");

    /// Return a list of edges whose destination vertex is v
    Edges inEdges(Vertex v, const QString &filter = "");

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

    //DEPRECATED:
    /// Update vertex's information with model's items' information
    void updateVertexInformation();

  private:
    Vertex vertex(DirectedGraph::VertexDescriptor vd);

    bool findRelation(const VertexDescriptor source,
                      const VertexDescriptor destination,
                      const QString         &relation,
                      OutEdgeIterator       &edge
                     ) const;

  private:
    mutable Graph m_graph;

    friend void IO::read(std::istream& stream, DirectedGraphSPtr graph, IO::PrintFormat format);
    friend void IO::write(const DirectedGraphSPtr graph, std::ostream& stream, IO::PrintFormat format);
  };
}

#endif // ESPINA_DIRECTED_GRAPH_H
