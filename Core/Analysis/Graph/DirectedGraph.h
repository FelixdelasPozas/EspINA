/*

    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>

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
#ifndef ESPINA_DIRECTED_GRAPH_H
#define ESPINA_DIRECTED_GRAPH_H

#include "Core/EspinaCore_Export.h"

// Boost
#include <boost/graph/adjacency_list.hpp>

// ESPINA
#include "Core/Types.h"
#include "Core/IO/GraphIO.h"

// Qt
#include <QTextStream>
#include <QMutex>

namespace ESPINA
{
  /** \brief Graph like structure which contains all the relationships
   *        between different elements of the model.
   */
  class EspinaCore_EXPORT DirectedGraph
  {
    public:
      using Vertex    = PersistentSPtr;
      using VertexPtr = PersistentPtr;

      struct EdgeProperty
      {
        std::string relationship;  /** relationship name .*/
      };

      /** \brief Relationships between analysis items connected by an edge
       *
       */
      struct Edge
      {
        Vertex      source;        /** origin of the relation.      */
        Vertex      target;        /** destination of the relation. */
        std::string relationship;  /** relation name.               */
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
      /** \brief DirectedGraph class constructor.
       *
       */
      explicit DirectedGraph();

      /** \brief DirectedGraph class destructor.
       *
       */
      ~DirectedGraph()
      {}

      /** \brief Remove all vertices and edges from the graph;
       *
       */
      void clear();

      /** \brief Add a vertex to the graph.
       * \param[in] vertex, vertex to add.
       *
       */
      void add(Vertex vertex);

      /** \brief Remove a vertex from the graph.
       * \param[in] vertex, vertex to remove.
       *
       */
      void remove(Vertex vertex);

      /** \brief Add given relation if relation doesn't already exists.
       * \param[in] ancestor, origin vertex.
       * \param[in] successor, destination vertex.
       * \param[in] description, description of the relation.
       *
       */
      void addRelation(Vertex ancestor,
                       Vertex successor,
                       const QString &description);

      /** \brief Remove given relation if it exists.
       * \param[in] ancestor, origin vertex.
       * \param[in] successor, destination vertex.
       * \param[in] description, description of the relation.
       *
       */
      void removeRelation(Vertex  ancestor,
                          Vertex  successor,
                          const QString &description);

      /** \brief Returns true if the graph contains the vertex.
       * \param[in] vertex, vertex to check.
       *
       */
      bool contains(Vertex vertex) const;

      /** \brief Returns all graph's edges that match the filter.
       * \param[in] filter, discrimination filter.
       *
       */
      Edges edges(const QString &filter = "") const;

      /** \brief Return a list of edges whose destination vertex is v.
       * \param[in] vertex, vertex to check.
       * \param[in] filter, discrimination filter.
       *
       */
      Edges inEdges(Vertex vertex, const QString &filter = QString()) const;

      /** \brief Return a list of edges whose destination vertex is v.
       * \param[in] vertex, raw pointer to a vertex to check.
       * \param[in] filter, discrimination filter.
       *
       */
      Edges inEdges(VertexPtr vertex, const QString &filter = QString()) const;

      ///
      /** \brief Return a list of edges whose source vertex is v.
       * \param[in] vertex, vertex to check.
       * \param[in] filter, discrimination filter.
       *
       */
      Edges outEdges(Vertex vertex, const QString &filter = QString()) const;

      /** \brief Return a list of edges whose source vertex is v.
       * \param[in] vertex, raw pointer to a vertex to check.
       * \param[in] filter, discrimination filter.
       *
       */
      Edges outEdges(VertexPtr vertex, const QString &filter = QString()) const;

      /** \brief Return a list of edges whose source or destination vertex is v.
       * \param[in] vertex, vertex to check.
       * \param[in] filter, discrimination filter.
       *
       */
      Edges edges(Vertex vertex, const QString &filter = QString()) const;

      /** \brief Remove all edges whose source or destination vertex is v.
       * \param[in] vertex, vertex to check.
       *
       */
      void removeEdges(Vertex vertex);

      /** \brief Return all graph's vertices.
       *
       */
      Vertices vertices() const;

      /** \brief Return all vertices whose outgoing edges end on v.
       * \param[in] vertex, vertex to check.
       * \param[in] filter, discrimination filter.
       *
       */
      Vertices ancestors(Vertex vertex, const QString &filter = QString()) const;

      /** \brief Return all vertices whose outgoing edges end on v.
       * \param[in] vertex, raw pointer to a vertex to check.
       * \param[in] filter, discrimination filter.
       *
       */
      Vertices ancestors(VertexPtr vertex, const QString &filter = QString()) const;

      /** \brief Return all vertices whose incoming edges start on v.
       * \param[in] vertex, vertex to check.
       * \param[in] filter, discrimination filter.
       *
       */
      Vertices successors(Vertex vertex, const QString &filter = QString()) const;

      /** \brief Return all vertices whose incoming edges start on v.
       * \param[in] vertex, raw pointer to a vertex to check.
       * \param[in] filter, discrimination filter.
       *
       */
      Vertices successors(VertexPtr vertex, const QString &filter = QString()) const;

      /** \brief Returns the vertices that are the ancestors of a given vertex in a given graph.
       * \param[in] vertex, vertex object to check.
       * \param[in] graph, directed graph smart pointer.
       *
       */
      DirectedGraph::Vertices rootAncestors(Vertex vertex) const;

      /** \brief Returns the vertices that are the ancestors of a given vertex in a given graph.
       * \param[in] vertex, raw pointer of the vertex to check.
       * \param[in] graph, directed graph smart pointer.
       *
       */
      DirectedGraph::Vertices rootAncestors(VertexPtr vertex) const;

    private:
      /** \brief Returns a vertex associated to a given descriptor.
       * \param[in] descriptor, vertex descriptor.
       *
       */
      DirectedGraph::Vertex vertex(VertexDescriptor descriptor) const;

      /** \brief Returns a descriptor associated to a given vertex.
       * \param[in] vertex, vertex object.
       *
       */
      DirectedGraph::VertexDescriptor descriptor(Vertex vertex) const;

      /** \brief Returns a descriptor associated to a given vertex.
       * \param[in] vertex, vertex raw pointer.
       *
       */
      DirectedGraph::VertexDescriptor descriptor(VertexPtr vertex) const;

      /** \brief Returns an edge iterator if the given relation between vertices is found.
       * \param[in] source, source vertex descriptor.
       * \param[in] destination, destination vertex descriptor.
       * \param[in] relation, relation description.
       *
       */
      DirectedGraph::OutEdgeIterator findRelation(const VertexDescriptor source,
                                                  const VertexDescriptor destination,
                                                  const QString         &relation) const;

      /** \brief Returns true if the graph contains the given vertex.
       * \param[in] vertex vertex to check for inclusion.
       *
       */
      bool contains_implementation(Vertex vertex) const;

      /** \brief Helper method to return a list of edges whose destination vertex is v.
       * \param[in] vertex, raw pointer to a vertex to check.
       * \param[in] filter, discrimination filter.
       *
       */
      Edges inEdges_implementation(VertexPtr vertex, const QString& filter) const;

      /** \brief Helper method that return a list of edges whose origin vertex is v.
       * \param[in] vertex, raw pointer to a vertex to check.
       * \param[in] filter, discrimination filter.
       *
       */
      Edges outEdges_implementation(VertexPtr vertex, const QString& filter = QString()) const;

      /** \brief Helper method to return all vertices whose outgoing edges end on v.
       * \param[in] vertex, raw pointer to a vertex to check.
       * \param[in] filter, discrimination filter.
       *
       */
      Vertices ancestors_implementation(VertexPtr vertex, const QString& filter = QString()) const;

    private:
      mutable Graph  m_graph;
      mutable QMutex m_mutex;

     friend void IO::Graph::read(std::istream& stream, DirectedGraphSPtr graph, IO::Graph::PrintFormat format);
     friend void IO::Graph::write(const DirectedGraphSPtr graph, std::ostream& stream, IO::Graph::PrintFormat format);
  };

  using DirectedGraphSPtr = std::shared_ptr<DirectedGraph>;
} // namespace ESPINA

#endif // ESPINA_DIRECTED_GRAPH_H
