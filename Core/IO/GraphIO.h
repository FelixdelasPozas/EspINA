/*
 *
 * Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 * This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef ESPINA_IO_GRAPH_GRAPHIO_H
#define ESPINA_IO_GRAPH_GRAPHIO_H

// C++
#include <iostream>
#include <cstdint>
#include <memory>

// ESPINA
#include <Core/Analysis/Persistent.h>

namespace ESPINA
{
  class DirectedGraph;
  using DirectedGraphSPtr = std::shared_ptr<DirectedGraph>;

  namespace IO
  {
    namespace Graph
    {
      enum class PrintFormat: std::int8_t { BOOST = 1, DEBUG = 2 };

      struct Unknown_Type_Found{};

      enum class VertexType: std::int8_t
      {
        SAMPLE = 1, FILTER = 2, CHANNEL = 3, SEGMENTATION = 4
      };

      class ReadOnlyVertex
      : public Persistent
      {
      public:
      	/* \brief ReadOnlyVertex class constructor.
      	 * \param[in] type, VertexType type.
      	 * \param[in] vertexid, id of the vertex.
      	 *
      	 */
        explicit ReadOnlyVertex(VertexType type, int vertexId)
        : m_type{type}, m_vertexId(vertexId) {}

        /* \brief Returns the type of the vertex.
         *
         */
        VertexType type() const
        { return m_type; }

        /* \brief Returns the id of the vertex.
         *
         */
        int vertexId() const
        { return m_vertexId; }

        /* \brief Implements Persistent::restoreState().
         *
         */
        virtual void restoreState(const State& state)
        { m_state = state; }

        /* \brief Implements Persistent::state().
         *
         */
        virtual State state() const
        { return m_state; }

        /* \brief Implements Persistent::snapshot().
         *
         */
        virtual Snapshot snapshot() const
        { return Snapshot(); }

        /* \brief Implements Persistent::unload().
         *
         */
        virtual void unload(){}

      private:
        VertexType m_type;
        int        m_vertexId;
        State      m_state;
      };

      /* \brief Reads and builds the graph from a byte stream.
       * \param[in] stream, graph to parse.
       * \param[out] graph, directed graph.
       * \param[in] format, print format specifier.
       *
       */
      void read (std::istream& stream, DirectedGraphSPtr graph, PrintFormat format = PrintFormat::BOOST);

      /* \brief Writes the graph to a byte stream.
       * \param[in] graph, directed graph.
       * \param[out] stream, graph to parse.
       * \param[in] format, print format specifier.
       *
       */
      void write(const DirectedGraphSPtr graph, std::ostream& stream, PrintFormat format = PrintFormat::BOOST);

      /* \brief Returns true if the vertex is the type SAMPLE.
       * \param[in] vertex.
       *
       */
      template<class T> bool isSample(const T &vertex)
      { return vertex->type() == VertexType::SAMPLE; }

      /* \brief Returns true if the vertex is the type FILTER.
       * \param[in] vertex.
       *
       */
      template<class T> bool isFilter(const T &vertex)
      { return vertex->type() == VertexType::FILTER; }

      /* \brief Returns true if the vertex is the type CHANNEL.
       * \param[in] vertex.
       *
       */
      template<class T> bool isChannel(const T &vertex)
      { return vertex->type() == VertexType::CHANNEL; }

      /* \brief Returns true if the vertex is the type SEGMENTATION.
       * \param[in] vertex.
       *
       */
      template<class T> bool isSegmentation(const T &vertex)
      { return vertex->type() == VertexType::SEGMENTATION; }
    }
  }
}

#endif // ESPINA_IO_GRAPH_GRAPHIO_H
