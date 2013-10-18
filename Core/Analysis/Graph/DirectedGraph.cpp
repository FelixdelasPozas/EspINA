/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) <year>  <name of author>

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

#include "DirectedGraph.h"

#include <iostream>
#undef foreach // Due to Qt-Boost incompatibility
#include <boost/graph/graphviz.hpp>
#include <boost/graph/adjacency_list_io.hpp>
#include <boost/algorithm/string.hpp>

#include <QDebug>

using namespace boost;
using namespace EspINA;

const std::string CHANNEL_TYPE      = "trapezium";
const std::string SEGMENTATION_TYPE = "ellipse";
const std::string FILTER_TYPE       = "box";
const std::string SAMPLE_TYPE       = "invtriangle";

namespace EspINA
{
  std::ostream& operator << ( std::ostream& out, const DirectedGraph::Vertex& v)
  {
//     out << v.descriptor << std::endl
//         << v.shape      << std::endl
//         << v.name       << std::endl
//         << v.args;
    return out;
}

std::istream& operator >> ( std::istream& in, DirectedGraph::Vertex& v)
{
//   const int MAX = 10000;
//   char buff[MAX];
//   in >> v.descriptor;
//   in >> v.shape;
//   in.getline(buff, 2);//Process shape's endl
//   in.getline(buff, MAX);
//   v.name = buff;
//   // a trailing space is added even there's no such a space
//   // on writing, then we have to trim it out
//   trim(v.name);
//   in.getline(buff, MAX);
//   v.args = buff;
//   trim(v.args);
//   v.item = AnalysisItemSPtr();
//   return in;
}

std::ostream& operator << ( std::ostream& out, const DirectedGraph::EdgeProperty& e )
{
//   out << e.relationship << " ";
  return out;
}

std::istream& operator >> ( std::istream& in, DirectedGraph::EdgeProperty& e)
{
//   in >> e.relationship;
  return in;
}
}

//-----------------------------------------------------------------------------
DirectedGraph::DirectedGraph()
: m_graph(0)
{
}

//-----------------------------------------------------------------------------
void DirectedGraph::addItem(AnalysisItemSPtr item)
{
  Q_ASSERT(vertex(item).item == nullptr);

  VertexDescriptor v = add_vertex(m_graph);

  m_graph[v].item       = item;
  m_graph[v].descriptor = v;
  //qDebug() << "New Vertex" << v << ":"  << item->data().toString();
  //qDebug() << item->data(Qt::DisplayRole) << " = " << v;
}

//-----------------------------------------------------------------------------
void DirectedGraph::removeItem(AnalysisItemSPtr item)
{
  Vertex v = vertex(item);

  clear_vertex (v.descriptor, m_graph);
  remove_vertex(v.descriptor, m_graph);
}

//-----------------------------------------------------------------------------
void DirectedGraph::updateVertexInformation()
{
//   int id = 0;
// 
//   VertexIterator vi, vi_end;
//   for(boost::tie(vi, vi_end) = boost::vertices(m_graph); vi != vi_end; vi++)
//   {
//     Vertex &vertex = m_graph[*vi];
//     AnalysisItemSPtr item = vertex.item;
//     Q_ASSERT(item);
//     if (!item)
//     {
//       qWarning() << "DirectedGraph: Trying to update invalid vertex";
//       continue;
//     }
//     switch (item->type())
//     {
//       case SAMPLE:
//         vertex.shape = SAMPLE_TYPE;
//         break;
//       case CHANNEL:
//         vertex.shape = CHANNEL_TYPE;
//         break;
//       case SEGMENTATION:
//       {
//         SegmentationPtr seg = segmentationPtr(item);
//         seg->updateCacheFlag();
//         vertex.shape = SEGMENTATION_TYPE;
//         break;
//       }
//       case FILTER:
//       {
//         FilterPtr filter = filterPtr(item);
//         filter->setId(id++);
//         //filter->resetCacheFlags();
//         vertex.shape = FILTER_TYPE;
//         break;
//       }
//       default:
//         Q_ASSERT(false);
//         break;
//     }
//     vertex.name       = item->data(Qt::DisplayRole).toString().toStdString();
//     vertex.args       = item->serialize().toStdString();
//     vertex.descriptor = *vi;
//   }
}

//-----------------------------------------------------------------------------
DirectedGraph::Vertex DirectedGraph::vertex(AnalysisItemSPtr item) const
{
  //   qDebug() << "Previous id" << item->m_vertex;
  Vertex v;

  VertexIterator vi, vi_end;
  boost::tie(vi, vi_end) = boost::vertices(m_graph);
  while(!v.item && vi != vi_end)
  {
    VertexDescriptor vd = *vi;

    if(m_graph[vd].item == item)
    {
      m_graph[vd].descriptor = vd;
      v = m_graph[vd];
    }

    ++vi;
  }

  return v;
}

//-----------------------------------------------------------------------------
DirectedGraph::Vertex DirectedGraph::vertex(DirectedGraph::VertexDescriptor vd)
{
  return m_graph[vd];
}

//-----------------------------------------------------------------------------
void DirectedGraph::addRelation(AnalysisItemSPtr   ancestor,
                                    AnalysisItemSPtr   successor,
                                    const QString &description)
{
//   EdgeProperty p;
//   p.relationship = description.toStdString();
// 
//   VertexDescriptor ancestorVD  = vertex(ancestor).descriptor;
//   VertexDescriptor successorVD = vertex(successor).descriptor;
// 
//   OutEdgeIterator edge;
//   if (findRelation(ancestorVD, successorVD, description, edge))
//   {
//     qWarning() << "DirectedGraph: Realtion (" << ancestor->data().toString() << "==>" << description << "==>" << successor->data().toString() << ") already exists";
//   } else
//   {
//     boost::add_edge(ancestorVD, successorVD, p, m_graph);
//   }
}

//-----------------------------------------------------------------------------
void DirectedGraph::removeRelation(AnalysisItemSPtr   ancestor,
                                       AnalysisItemSPtr   successor,
                                       const QString &description)
{
//   VertexDescriptor ancestorVD  = vertex(ancestor).descriptor;
//   VertexDescriptor successorVD = vertex(successor).descriptor;
// 
//   OutEdgeIterator edge;
//   if (findRelation(ancestorVD, successorVD, description, edge))
//   {
//     boost::remove_edge(edge, m_graph);
//   } else
//   {
//     qWarning() << "DirectedGraph: Realtion (" << ancestor->data().toString() << "==>" << description << "==>" << successor->data().toString() << ") already removed";
//   }
}


//-----------------------------------------------------------------------------
DirectedGraph::Edges DirectedGraph::edges(const QString &filter)
{
  Edges result;

  EdgeIterator ei, ei_end;
  for(boost::tie(ei, ei_end) = boost::edges(m_graph); ei != ei_end; ei++)
  {
    if (filter.isEmpty() || m_graph[*ei].relationship == filter.toStdString())
    {
      Edge e;
      e.source = m_graph[source(*ei, m_graph)];
      e.target = m_graph[target(*ei, m_graph)];
      e.relationship = m_graph[*ei].relationship;
      result << e;
    }
  }
  return result;
}

//-----------------------------------------------------------------------------
DirectedGraph::Edges DirectedGraph::inEdges(Vertex v, const QString &filter)
{
  Edges result;

  InEdgeIterator ei, ei_end;
  for(boost::tie(ei, ei_end) = boost::in_edges(v.descriptor, m_graph); ei != ei_end; ei++)
  {
    if (filter.isEmpty() || m_graph[*ei].relationship == filter.toStdString())
    {
      Edge e;
      e.source = m_graph[source(*ei, m_graph)];
      e.target = m_graph[target(*ei, m_graph)];
      e.relationship = m_graph[*ei].relationship;
      result << e;
    }
  }
  return result;
}

//-----------------------------------------------------------------------------
DirectedGraph::Edges DirectedGraph::outEdges(Vertex v, const QString &filter)
{
  Edges result;

  OutEdgeIterator ei, ei_end;
  for(boost::tie(ei, ei_end) = boost::out_edges(v.descriptor, m_graph); ei != ei_end; ei++)
  {
    if (filter.isEmpty() || m_graph[*ei].relationship == filter.toStdString())
    {
      Edge e;
      e.source = m_graph[source(*ei, m_graph)];
      e.target = m_graph[target(*ei, m_graph)];
      e.relationship = m_graph[*ei].relationship;
      result << e;
    }
  }
  return result;
}

//-----------------------------------------------------------------------------
DirectedGraph::Edges DirectedGraph::edges(Vertex v, const QString &filter)
{
  Edges result;

  result << inEdges (v, filter);
  result << outEdges(v, filter);

  return result;
}

//-----------------------------------------------------------------------------
void DirectedGraph::removeEdges(Vertex v)
{
  OutEdgeIterator oei, oei_end;
  boost::tie(oei, oei_end) = boost::out_edges(v.descriptor, m_graph); 
  while(oei != oei_end)
  {
    boost::remove_edge(oei, m_graph);
    boost::tie(oei, oei_end) = boost::out_edges(v.descriptor, m_graph); 
  }

  Vertices ancestorList = ancestors(v);
  for (int i = 0; i < ancestorList.size(); i++)
  {
    VertexDescriptor ancestorId = ancestorList[i].descriptor;
    boost::tie(oei, oei_end) = boost::out_edges(ancestorId, m_graph); 
    while(oei != oei_end)
    {
      if (target(*oei, m_graph) == v.descriptor)
      {
        boost::remove_edge(oei, m_graph);
        boost::tie(oei, oei_end) = boost::out_edges(ancestorId, m_graph); 
      } else
        oei++;
    }
  }
}

//-----------------------------------------------------------------------------
DirectedGraph::Vertices DirectedGraph::vertices() const
{
  Vertices result;

  VertexIterator vi, vi_end;
  for(boost::tie(vi, vi_end) = boost::vertices(m_graph); vi != vi_end; vi++)
  {
    //     qDebug() << *vi << m_graph[*vi].name.c_str() << m_graph[*vi].args.c_str();
    //     Q_ASSERT(m_graph[*vi].descriptor == *vi);
    m_graph[*vi].descriptor = *vi;
    result << m_graph[*vi];
  }

  return result;
}

//-----------------------------------------------------------------------------
DirectedGraph::Vertices DirectedGraph::ancestors(Vertex v, const QString &filter) const
{
  Vertices result;
  InEdgeIterator iei, iei_end;

  //   qDebug() << "Ancestors of:" << m_graph[v].name.c_str();
  for(boost::tie(iei, iei_end) = boost::in_edges(v.descriptor, m_graph); iei != iei_end; iei++)
  {
    //     qDebug() << "\t" << source(*iei, m_graph) << m_graph[source(*iei,m_graph)].name.c_str();
    if (filter.isEmpty() || m_graph[*iei].relationship == filter.toStdString())
    {
      //       qDebug() << "Pass Filter:"  << m_graph[source(*iei, m_graph)].descriptor << m_graph[source(*iei, m_graph)].name.c_str();
      VertexDescriptor v = source(*iei, m_graph);
      m_graph[v].descriptor = v;
      result << m_graph[v];
    }
  }
  return result;
}

//-----------------------------------------------------------------------------
DirectedGraph::Vertices DirectedGraph::succesors(Vertex v, const QString &filter) const
{
  Vertices result;
  OutEdgeIterator oei, oei_end;

  //   qDebug() << "Successors of:" << m_graph[v].name.c_str();
  for(boost::tie(oei, oei_end) = boost::out_edges(v.descriptor, m_graph); oei != oei_end; oei++)
  {
    //     qDebug() << "\t" << m_graph[target(*oei,m_graph)].name.c_str();
    if (filter.isEmpty() || m_graph[*oei].relationship == filter.toStdString())
    {
      VertexDescriptor v = target(*oei, m_graph);
      m_graph[v].descriptor = v;
      result << m_graph[v];
    }
  }
  return result;
}

// //-----------------------------------------------------------------------------
// void DirectedGraph::setItem(Vertex &v, AnalysisItemSPtr item)
// {
//   Q_ASSERT(item);
//   v.item = item;
// 
//   m_graph[v.descriptor].item = v.item;
// }

// //-----------------------------------------------------------------------------
// AnalysisItemType DirectedGraph::type(const Vertex v)
// {
//   if (v.item)
//     return v.item->type();
//   else if (v.shape == SAMPLE_TYPE)
//     return SAMPLE;
//   else if (v.shape == CHANNEL_TYPE)
//     return CHANNEL;
//   else if (v.shape == SEGMENTATION_TYPE)
//     return SEGMENTATION;
//   else if (v.shape == FILTER_TYPE)
//     return FILTER;
// 
//   Q_ASSERT(false);
//   return TAXONOMY;
// }

//-----------------------------------------------------------------------------
void DirectedGraph::read(std::istream& stream, DirectedGraph::PrintFormat format)
{
  switch (format)
  {
    case BOOST:
      stream >> boost::read(m_graph);
      break;
    case GRAPHVIZ:
    {
      boost::dynamic_properties dp;

      dp.property("node_id", boost::get(boost::vertex_index        , m_graph));
//       dp.property("label"  , boost::get(&Vertex::name      , m_graph));
//       dp.property("shape"  , boost::get(&Vertex::shape     , m_graph));
//       dp.property("args"   , boost::get(&Vertex::args      , m_graph));
      dp.property("label"  , boost::get(&EdgeProperty::relationship, m_graph));
      boost::read_graphviz(stream, m_graph, dp);

      break;
    }
    default:
      qWarning("Format Unkown");
      break;
  };

}

//-----------------------------------------------------------------------------
void DirectedGraph::write(std::ostream &stream, DirectedGraph::PrintFormat format)
{
  switch (format)
  {
    case BOOST:
      stream << boost::write(m_graph) << std::endl;
      //       std::cout << boost::write(m_graph) << std::endl;
      break;
    case GRAPHVIZ:
    {
      boost::dynamic_properties dp;

      dp.property("node_id", boost::get(boost::vertex_index        , m_graph));
//       dp.property("label"  , boost::get(&Vertex::name      , m_graph));
//       dp.property("shape"  , boost::get(&Vertex::shape     , m_graph));
//       dp.property("args"   , boost::get(&Vertex::args      , m_graph));
      dp.property("label"  , boost::get(&EdgeProperty::relationship, m_graph));
      boost::write_graphviz_dp(stream, m_graph, dp);

      break;
    }
    default:
      qWarning("Format Unkown");
      break;
  };
}

//-----------------------------------------------------------------------------
bool DirectedGraph::findRelation(const VertexDescriptor source,
                                     const VertexDescriptor destination,
                                     const QString         &relation,
                                     OutEdgeIterator       &edge) const
{
  bool found = false;
  OutEdgeIterator oei_end;

  boost::tie(edge, oei_end) = boost::out_edges(source, m_graph);
  while (!found && edge != oei_end)
  {
    found  = target(*edge, m_graph)      == destination;
    found &= m_graph[*edge].relationship == relation.toStdString();
    if (!found)
      edge++;
  }

  return found;
}
