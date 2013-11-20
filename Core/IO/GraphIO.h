/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2013  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 * This program is free software: you can redistribute it and/or modify
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

namespace EspINA
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
        SAMPLE, FILTER, CHANNEL, SEGMENTATION, EXTENSION_PROVIDER
      };

      class ReadOnlyVertex
      : public Persistent
      {
      public:
        explicit ReadOnlyVertex(VertexType type)
        : m_type{type}{}

        VertexType type() const
        { return m_type; }

        virtual void restoreState(const State& state)
        { m_state = state; }
        virtual State state() const
        { return m_state; }
        virtual Snapshot snapshot() const
        { return Snapshot(); }
        virtual void unload(){}

      private:
        VertexType m_type;
        State      m_state;
      };

      void read (std::istream& stream, DirectedGraphSPtr graph, PrintFormat format = PrintFormat::BOOST);
      void write(const DirectedGraphSPtr graph, std::ostream& stream, PrintFormat format = PrintFormat::BOOST);

//       class GraphIO
//       {
//       public:
//         virtual ~GraphIO() {}
// 
//         void read (std::istream& stream, DirectedGraphSPtr graph, PrintFormat format = PrintFormat::BOOST);
// 
//         void write(const DirectedGraphSPtr graph, std::ostream& stream, PrintFormat format = PrintFormat::BOOST);
// 
//       protected:
// //         virtual void readBoost (std::istream& stream, DirectedGraph::Graph& graph) = 0;
// //         virtual void writeBoost(DirectedGraph::Graph& graph, std::ostream& stream) = 0;
//         friend std::ostream& operator << (std::ostream& out, const DirectedGraph::Vertex& v)
//         {
//           out << v.descriptor << std::endl;
//           //out << shape(v.item) << std::endl;
//           //         << v.shape      << std::endl
//           //         << v.name       << std::endl
//           //         << v.args;
//           return out;
//         }
//         friend std::istream& operator >> ( std::istream& in, DirectedGraph::Vertex& v)
//         {
//           return in;
//         }
//         friend std::ostream& operator << ( std::ostream& out, const DirectedGraph::EdgeProperty& e )
//         {
//           out << e.relationship << " ";
//           return out;
//         }
// 
//         friend std::istream& operator >> ( std::istream& in, DirectedGraph::EdgeProperty& e)
//         {
//           //   in >> e.relationship;
//           return in;
//         }
//       };
    }
  }
}

#endif // ESPINA_IO_GRAPH_GRAPHIO_H