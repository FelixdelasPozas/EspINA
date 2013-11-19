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
#include "GraphIO.h"

#include <boost/graph/adjacency_list_io.hpp>
#include "Core/Analysis/Graph/DirectedGraph.h"
#include "Core/Analysis/Sample.h"
#include "Core/Analysis/Channel.h"
#include "Core/Analysis/Filter.h"
#include "Core/Analysis/Segmentation.h"
#include "Core/Analysis/Extensions/ExtensionProvider.h"

using namespace EspINA::IO;
using namespace EspINA::IO::Graph;

namespace EspINA {

  const std::string CHANNEL_TYPE            = "trapezium";
  const std::string SEGMENTATION_TYPE       = "ellipse";
  const std::string FILTER_TYPE             = "box";
  const std::string SAMPLE_TYPE             = "invtriangle";
  const std::string EXTENSION_PROVIDER_TYPE = "diamond";

  std::string type(const PersistentSPtr item)
  {
    if (dynamic_cast<SamplePtr>(item.get()))
    {
      return SAMPLE_TYPE;
    } else if (dynamic_cast<ChannelPtr>(item.get()))
    {
      return CHANNEL_TYPE;
    } else if (dynamic_cast<FilterPtr>(item.get()))
    {
      return FILTER_TYPE;
    } else if (dynamic_cast<SegmentationPtr>(item.get()))
    {
      return SEGMENTATION_TYPE;
    } else if (dynamic_cast<ExtensionProviderPtr>(item.get()))
    {
      return EXTENSION_PROVIDER_TYPE;
    } else {
      throw Unknown_Type_Found();
    }
  }

  std::ostream& operator<<(std::ostream& out, const DirectedGraph::Vertex& v)
  {
    out << v->uuid().toString().toStdString() << std::endl;
    out << type(v) << std::endl;
    out << v->name().toStdString() << std::endl;
    out << v->saveState().toStdString();

    return out;
  }

  VertexType vertexType(const std::string& shape)
  {
    if (shape == SAMPLE_TYPE)
    {
      return VertexType::SAMPLE;
    } else if (shape == CHANNEL_TYPE)
    {
      return VertexType::CHANNEL;
    } else if (shape == FILTER_TYPE)
    {
      return VertexType::FILTER;
    } else if (shape == SEGMENTATION_TYPE)
    {
      return VertexType::SEGMENTATION;
    } else if (shape == EXTENSION_PROVIDER_TYPE)
    {
      return VertexType::EXTENSION_PROVIDER;
    } else {
      throw Unknown_Type_Found();
    }
  }


  std::istream& operator>> (std::istream& in, DirectedGraph::Vertex& v)
  {
    const int MAX = 10000;
    char buff[MAX];

    in.getline(buff, MAX);
    QString uuid(buff);

    std::string type;
    in >> type;
    in.getline(buff, 2);//Consume type's endl

    in.getline(buff, MAX);
    QString name(buff);

    in.getline(buff, MAX);
    State state(buff);

    v = PersistentSPtr{new ReadOnlyVertex(vertexType(type))};
    v->setName(name);
    v->setUuid(uuid);
    v->restoreState(state);

    return in;
  }

  std::ostream& operator<<(std::ostream& out, const DirectedGraph::EdgeProperty& e)
  {
    out << e.relationship << " ";
    return out;
  }

  std::istream& operator>>(std::istream& in, DirectedGraph::EdgeProperty& e)
  {
    in >> e.relationship;
    return in;
  }
}

void Graph::read(std::istream& stream, DirectedGraphSPtr graph, PrintFormat format)
{
  if (format == PrintFormat::BOOST) {
    stream >> boost::read(graph->m_graph);
  }
}

void Graph::write(const DirectedGraphSPtr graph, std::ostream& stream, PrintFormat format)
{
  if (format == PrintFormat::BOOST) {
    stream << boost::write(graph->m_graph) << std::endl;
  }
}