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

#include "RelationshipGraph.h"

#include "Core/Model/Filter.h"
#include "Core/Model/ModelItem.h"
#include "Core/Model/Segmentation.h"

#include <iostream>
#undef foreach // Due to Qt-Boost incompatibility
#include <boost/graph/graphviz.hpp>
#include <boost/graph/adjacency_list_io.hpp>
#include <boost/algorithm/string.hpp>

#include <QDebug>

using namespace boost;
using namespace EspINA;

const std::string CHANNEL_SHAPE = "trapezium";
const std::string SEGMENTATION_SHAPE = "ellipse";
const std::string FILTER_SHAPE = "box";
const std::string SAMPLE_SHAPE = "invtriangle";

namespace EspINA
{
std::ostream& operator << ( std::ostream& out, const VertexProperty& v)
{
  out << v.vId   << std::endl
      << v.shape << std::endl
      << v.name  << std::endl
      << v.args;
  return out;
}

std::istream& operator >> ( std::istream& in, VertexProperty& v)
{
  const int MAX = 10000;
  char buff[MAX];
  in >> v.vId;
  in >> v.shape;
  in.getline(buff, 2);//Process shape's endl
  in.getline(buff, MAX);
  v.name = buff;
  // a trailing space is added even there's no such a space
  // on writing, then we have to trim it out
  trim(v.name);
  in.getline(buff, MAX);
  v.args = buff;
  trim(v.args);
  v.item = ModelItemPtr();
  return in;
}

std::ostream& operator << ( std::ostream& out, const RelationshipGraph::EdgeProperty& e )
{
  out << e.relationship << " ";
  return out;
}

std::istream& operator >> ( std::istream& in, RelationshipGraph::EdgeProperty& e)
{
  in >> e.relationship;
  return in;
}
}

//-----------------------------------------------------------------------------
/// Visit nodes by edges and return the root vertex id
template<class Graph, class VertexId>
QList<VertexId> rootVertices(const Graph& graph)
{
  VertexId v;
  QList<VertexId> discardedVertices, posibleRootVertices;

  typename boost::graph_traits<Graph>::edge_iterator ei, ei_end;
  for( boost::tie(ei, ei_end) = boost::edges(graph); ei != ei_end; ei++)
  {
    //       std::cout << *ei;
    v = boost::source(*ei, graph);
    //       std::cout << " v" << v;
    if( !discardedVertices.contains(v) && !posibleRootVertices.contains(v)){
      //         std::cout << " a posible root vertex ";
      posibleRootVertices.push_back(v);
    }
    //       std::cout << std::endl;
    v = boost::target(*ei, graph);
    if( posibleRootVertices.contains(v) ){
      //         std::cout << "v" << v << " deleted from posibleRootVertices "<< std::endl;
      posibleRootVertices.removeOne(v);
    }
    discardedVertices.push_back(v);

    //       qDebug() << "Discarded" << discardedVertices;
    //       qDebug() << "Posible Root" << posibleRootVertices;

  }
  // If there is no edges //TODO what if it is a closed graph?
  if( posibleRootVertices.size() == 0)
  {
    typename boost::graph_traits<Graph>::vertex_iterator vi, vi_end;
    for(boost::tie(vi, vi_end) = boost::vertices(graph); vi != vi_end; vi++)
    {
      posibleRootVertices.push_back(*vi);
    }
  }
  //     qDebug() << "Number of Root vertex " << posibleRootVertices.size();
  assert(posibleRootVertices.size() >= 1 );//? posibleRootVertices.at(0) : -1;
  return posibleRootVertices;

}

//-----------------------------------------------------------------------------
//! Retrieve a map of the parents or predecessors of all the vertex in graph
template<class Graph, class VertexId>
QMap<VertexId, QList<VertexId> > predecessors(const Graph& g)
{
  QMap<VertexId, QList<VertexId> > res;
  QList<VertexId> list;
  VertexId vs, vt;

  typename boost::graph_traits<Graph>::edge_iterator ei, ei_end;
  for( boost::tie(ei, ei_end) = boost::edges(g); ei != ei_end; ei++)
  {
    vs = boost::source(*ei, g);
    vt = boost::target(*ei, g);

    if( !res.contains(vt) ){
      list.clear();// = QList<VertexId>();
    }
    else
      list = res.value(vt);
    list.append(vs);
    res.insert(vt, list);
    //std::cout << vs << " (parentof) " << vt << std::endl;
  }
  return res;
}


//-----------------------------------------------------------------------------
RelationshipGraph::RelationshipGraph()
: m_graph(0)
{
  // m_graph.m_property.owner = name;
}

//-----------------------------------------------------------------------------
void RelationshipGraph::addItem(ModelItemPtr item)
{
  // TODO: Check if item's been already added to the graph
  VertexId v = add_vertex(m_graph);
  m_graph[v].item = item;
  //   qDebug() << item->data(Qt::DisplayRole) << " = " << v;
}

//-----------------------------------------------------------------------------
void RelationshipGraph::removeItem(ModelItemPtr item)
{
  VertexDescriptor itemVertex = vertex(item);
  clear_vertex (itemVertex, m_graph);
  remove_vertex(itemVertex, m_graph);
}

//-----------------------------------------------------------------------------
void RelationshipGraph::updateVertexInformation()
{
  int id = 0;

  VertexIterator vi, vi_end;
  for(boost::tie(vi, vi_end) = boost::vertices(m_graph); vi != vi_end; vi++)
  {
    VertexProperty &vertex = m_graph[*vi];
    ModelItemPtr item = vertex.item;
    if (!item)
      continue;
    Q_ASSERT(item);
    //vertex.vId  = item->m_vertex;
    switch (item->type())
    {
      case SAMPLE:
        vertex.shape = SAMPLE_SHAPE;
        break;
      case CHANNEL:
        vertex.shape = CHANNEL_SHAPE;
        break;
      case SEGMENTATION:
      {
        SegmentationPtr seg = segmentationPtr(item);
        seg->updateCacheFlag();
        vertex.shape = SEGMENTATION_SHAPE;
        break;
      }
      case FILTER:
      {
        FilterPtr filter = filterPtr(item);
        filter->setId(id++);
        filter->updateCacheFlags();
        vertex.shape = FILTER_SHAPE;
        break;
      }
      default:
        Q_ASSERT(false);
    }
  }
  // We need to do it in two loops to ensure cache flags are set properly
  for(boost::tie(vi, vi_end) = boost::vertices(m_graph); vi != vi_end; vi++)
  {
    VertexProperty &vertex = m_graph[*vi];
    ModelItemPtr item = vertex.item;
    if (!item)
      continue;
    Q_ASSERT(item);
    vertex.name = item->data(Qt::DisplayRole).toString().toStdString();
    vertex.args = item->serialize().toStdString();
  }
}

//-----------------------------------------------------------------------------
RelationshipGraph::VertexDescriptor RelationshipGraph::vertex(ModelItemPtr item)
{
  //   qDebug() << "Previous id" << item->m_vertex;
  VertexIterator vi, vi_end;
  for(boost::tie(vi, vi_end) = boost::vertices(m_graph); vi != vi_end; vi++)
  {
    if( m_graph[*vi].item == item )
      return *vi;
  }

  Q_ASSERT(false);
  return *vi;
}

//-----------------------------------------------------------------------------
EspINA::RelationshipGraph::VertexDescriptor RelationshipGraph::vertex(EspINA::RelationshipGraph::VertexId v)
{
  VertexIterator vi, vi_end;
  for(boost::tie(vi, vi_end) = boost::vertices(m_graph); vi != vi_end; vi++)
  {
    if( m_graph[*vi].vId == v )
      return *vi;
  }

  Q_ASSERT(false);
  return *vi;
}

//-----------------------------------------------------------------------------
void RelationshipGraph::addRelation(ModelItemPtr   ancestor,
                                    ModelItemPtr   successor,
                                    const QString &description)
{
  EdgeProperty p;
  p.relationship = description.toStdString();
  boost::add_edge(vertex(ancestor), vertex(successor), p, m_graph);
}

//-----------------------------------------------------------------------------
void RelationshipGraph::removeRelation(ModelItemPtr   ancestor,
                                       ModelItemPtr   successor,
                                       const QString &description)
{
  OutEdgeIterator oei, oei_end;

  //   qDebug() << "Ancestors of:" << m_graph[v].name.c_str();
  for(boost::tie(oei, oei_end) = boost::out_edges(vertex(ancestor), m_graph); oei != oei_end; oei++)
  {
    //     qDebug() << m_graph[*oei].relationship.c_str();
    if (target(*oei, m_graph) == vertex(successor) &&
      m_graph[*oei].relationship == description.toStdString())
    {
      OutEdgeIterator old = oei++;
      boost::remove_edge(old, m_graph);
    }
  }
}

//-----------------------------------------------------------------------------
void RelationshipGraph::connect(const QString &ancestor,
                                ModelItemPtr   successor,
                                const QString &description)
{
  VertexIterator vi, vi_end;
  for(boost::tie(vi, vi_end) = boost::vertices(m_graph); vi != vi_end; vi++)
  {
    Q_ASSERT(false);
  }
  addRelation(m_graph[*vi].item, successor, description);
}

//-----------------------------------------------------------------------------
Edges RelationshipGraph::edges(const QString &filter)
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
Edges RelationshipGraph::inEdges(RelationshipGraph::VertexId v,
                                 const QString &filter)
{
  Edges result;

  InEdgeIterator ei, ei_end;
  for(boost::tie(ei, ei_end) = boost::in_edges(v, m_graph); ei != ei_end; ei++)
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
Edges RelationshipGraph::outEdges(RelationshipGraph::VertexId v,
                                  const QString &filter)
{
  Edges result;

  OutEdgeIterator ei, ei_end;
  for(boost::tie(ei, ei_end) = boost::out_edges(v, m_graph); ei != ei_end; ei++)
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
Edges RelationshipGraph::edges(RelationshipGraph::VertexId v,
                               const QString &filter)
{
  Edges result;

  result << inEdges (v, filter);
  result << outEdges(v, filter);

  return result;
}

//-----------------------------------------------------------------------------
void RelationshipGraph::removeEdges(RelationshipGraph::VertexId v)
{
  OutEdgeIterator oei, oei_end;
  boost::tie(oei, oei_end) = boost::out_edges(v, m_graph); 
  while(oei != oei_end)
  {
    boost::remove_edge(oei, m_graph);
    boost::tie(oei, oei_end) = boost::out_edges(v, m_graph); 
  }

  Vertices ancestorList = ancestors(v);
  for (int i = 0; i < ancestorList.size(); i++)
  {
    VertexId ancestorId = ancestorList[i].vId;
    boost::tie(oei, oei_end) = boost::out_edges(ancestorId, m_graph); 
    while(oei != oei_end)
    {
      if (target(*oei, m_graph) == vertex(v))
      {
        boost::remove_edge(oei, m_graph);
        boost::tie(oei, oei_end) = boost::out_edges(ancestorId, m_graph); 
      } else
        oei++;
    }
  }
}

//-----------------------------------------------------------------------------
Vertices RelationshipGraph::vertices()
{
  Vertices result;

  VertexIterator vi, vi_end;
  for(boost::tie(vi, vi_end) = boost::vertices(m_graph); vi != vi_end; vi++)
  {
    //     qDebug() << *vi << m_graph[*vi].name.c_str() << m_graph[*vi].args.c_str();
    //     Q_ASSERT(m_graph[*vi].vId == *vi);
    m_graph[*vi].vId = *vi;
    result << m_graph[*vi];
  }

  return result;
}

//-----------------------------------------------------------------------------
Vertices RelationshipGraph::ancestors(RelationshipGraph::VertexId v,
                                      const QString &filter)
{
  Vertices result;
  InEdgeIterator iei, iei_end;

  //   qDebug() << "Ancestors of:" << m_graph[v].name.c_str();
  for(boost::tie(iei, iei_end) = boost::in_edges(v, m_graph); iei != iei_end; iei++)
  {
    //     qDebug() << "\t" << source(*iei, m_graph) << m_graph[source(*iei,m_graph)].name.c_str();
    if (filter.isEmpty() || m_graph[*iei].relationship == filter.toStdString())
    {
      //       qDebug() << "Pass Filter:"  << m_graph[source(*iei, m_graph)].vId << m_graph[source(*iei, m_graph)].name.c_str();
      VertexDescriptor v = source(*iei, m_graph);
      //       m_graph[v].vId = v;
      result << m_graph[v];
    }
  }
  return result;
}

//-----------------------------------------------------------------------------
Vertices RelationshipGraph::succesors(RelationshipGraph::VertexId v,
                                      const QString &filter)
{
  Vertices result;
  OutEdgeIterator oei, oei_end;

  //   qDebug() << "Successors of:" << m_graph[v].name.c_str();
  for(boost::tie(oei, oei_end) = boost::out_edges(v, m_graph); oei != oei_end; oei++)
  {
    //     qDebug() << "\t" << m_graph[target(*oei,m_graph)].name.c_str();
    if (filter.isEmpty() || m_graph[*oei].relationship == filter.toStdString())
    {
      VertexDescriptor v = target(*oei, m_graph);
      //       m_graph[v].vId = v;
      result << m_graph[v];
    }
  }
  return result;
}


//-----------------------------------------------------------------------------
bool RelationshipGraph::find(VertexProperty vp, VertexProperty &foundV)
{
  VertexIterator vi, vi_end;
  for(boost::tie(vi, vi_end) = boost::vertices(m_graph); vi != vi_end; vi++)
  {
    if (m_graph[*vi].name  == vp.name  &&
      m_graph[*vi].shape == vp.shape &&
      m_graph[*vi].args  == vp.args)
    {
      foundV = m_graph[*vi];
      return true;
    }
  }
  return false;
}

//-----------------------------------------------------------------------------
void RelationshipGraph::setItem(RelationshipGraph::VertexId v, ModelItemPtr item)
{
  m_graph[v].item = item;
}


//-----------------------------------------------------------------------------
QString RelationshipGraph::name(RelationshipGraph::VertexId v) const
{
  return QString(m_graph[v].name.c_str());
}

//-----------------------------------------------------------------------------
ModelItemType RelationshipGraph::type(const VertexProperty v)
{
  if (v.item)
    return v.item->type();
  else if (v.shape == SAMPLE_SHAPE)
    return SAMPLE;
  else if (v.shape == CHANNEL_SHAPE)
    return CHANNEL;
  else if (v.shape == SEGMENTATION_SHAPE)
    return SEGMENTATION;
  else if (v.shape == FILTER_SHAPE)
    return FILTER;

  Q_ASSERT(false);
  return TAXONOMY;
}

//-----------------------------------------------------------------------------
QString RelationshipGraph::args(RelationshipGraph::VertexId v) const
{
  return QString(m_graph[v].args.c_str());
}

//-----------------------------------------------------------------------------
VertexProperty RelationshipGraph::properties(RelationshipGraph::VertexId v)
{
  return m_graph[v];
}


//-----------------------------------------------------------------------------
void RelationshipGraph::load(QTextStream& serialization)
{
  boost::dynamic_properties dp;

  dp.property("node_id", boost::get(&VertexProperty::vId       , m_graph));
  dp.property("label"  , boost::get(&VertexProperty::name      , m_graph));
  dp.property("shape"  , boost::get(&VertexProperty::shape     , m_graph));
  dp.property("args"   , boost::get(&VertexProperty::args      , m_graph));
  dp.property("label"  , boost::get(&EdgeProperty::relationship, m_graph));

  boost::read_graphviz(serialization.string()->toStdString(), m_graph, dp);
}

//-----------------------------------------------------------------------------
void RelationshipGraph::read(std::istream& stream, RelationshipGraph::PrintFormat format)
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
      dp.property("label"  , boost::get(&VertexProperty::name      , m_graph));
      dp.property("shape"  , boost::get(&VertexProperty::shape     , m_graph));
      dp.property("args"   , boost::get(&VertexProperty::args      , m_graph));
      dp.property("label"  , boost::get(&EdgeProperty::relationship, m_graph));
      boost::read_graphviz(stream, m_graph, dp);

      break;
    }
    default:
      qWarning("Format Unkown");
  };

}

//-----------------------------------------------------------------------------
void RelationshipGraph::write(std::ostream &stream, RelationshipGraph::PrintFormat format)
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
      dp.property("label"  , boost::get(&VertexProperty::name      , m_graph));
      dp.property("shape"  , boost::get(&VertexProperty::shape     , m_graph));
      dp.property("args"   , boost::get(&VertexProperty::args      , m_graph));
      dp.property("label"  , boost::get(&EdgeProperty::relationship, m_graph));
      boost::write_graphviz_dp(stream, m_graph, dp);

      break;
    }
    default:
      qWarning("Format Unkown");
  };
}