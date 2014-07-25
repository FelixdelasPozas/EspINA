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

#include <iostream>
#include <memory>
#include <Core/Analysis/Persistent.h>

namespace ESPINA
{
  class DirectedGraph;
  using DirectedGraphSPtr = std::shared_ptr<DirectedGraph>;

  namespace IO
  {
    namespace Graph
    {
      enum class PrintFormat
      { BOOST
      , DEBUG
      };

      struct Unknown_Type_Found{};

      enum class VertexType
      {
        SAMPLE, FILTER, CHANNEL, SEGMENTATION
      };

      class ReadOnlyVertex
      : public Persistent
      {
      public:
        explicit ReadOnlyVertex(VertexType type, int vertexId)
        : m_type{type}, m_vertexId(vertexId) {}

        VertexType type() const
        { return m_type; }

        int vertexId() const
        { return m_vertexId; }

        virtual void restoreState(const State& state)
        { m_state = state; }

        virtual State state() const
        { return m_state; }

        virtual Snapshot snapshot() const
        { return Snapshot(); }

        virtual void unload(){}

      private:
        VertexType m_type;
        int        m_vertexId;
        State      m_state;
      };

      void read (std::istream& stream, DirectedGraphSPtr graph, PrintFormat format = PrintFormat::BOOST);
      void write(const DirectedGraphSPtr graph, std::ostream& stream, PrintFormat format = PrintFormat::BOOST);

      template<class T> bool isSample(const T &vertex)
      { return vertex->type() == VertexType::SAMPLE; }

      template<class T> bool isFilter(const T &vertex)
      { return vertex->type() == VertexType::FILTER; }

      template<class T> bool isChannel(const T &vertex)
      { return vertex->type() == VertexType::CHANNEL; }

      template<class T> bool isSegmentation(const T &vertex)
      { return vertex->type() == VertexType::SEGMENTATION; }
    }
  }
}

#endif // ESPINA_IO_GRAPH_GRAPHIO_H