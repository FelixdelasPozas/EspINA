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

#include "common/model/Filter.h"
#include "common/model/ModelItem.h"

#include <iostream>
#include <boost/graph/graphviz.hpp>
#include <boost/graph/adjacency_list_io.hpp>

#include <QDebug>

using namespace boost;

const std::string BOX = "box";
const std::string ELLIPSE = "ellipse";
const std::string INVTRIANGLE = "invtriangle";
const std::string TRAPEZIUM = "trapezium";

std::ostream& operator << ( std::ostream& out, const VertexProperty& v)
{
  out << v.vId << std::endl
      << v.shape << std::endl
      << v.name << std::endl
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
  in.getline(buff, MAX);
  v.args = buff;
  v.item = NULL;
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
void RelationshipGraph::addItem(ModelItem* item)
{
  // TODO: Check if item's been already added to the graph
  VertexId v = add_vertex(m_graph);
  m_graph[v].item = item;
  item->m_vertex = v;
  item->m_relations = this;
//   qDebug() << item->data(Qt::DisplayRole) << " = " << v;
}

//-----------------------------------------------------------------------------
void RelationshipGraph::removeItem(ModelItem *item)
{
  VertexDescriptor itemVertex = vertex(item);
  clear_vertex (itemVertex, m_graph);
  remove_vertex(itemVertex, m_graph);
}

//-----------------------------------------------------------------------------
void RelationshipGraph::updateVertexInformation()
{
  VertexIterator vi, vi_end;
  for(boost::tie(vi, vi_end) = boost::vertices(m_graph); vi != vi_end; vi++)
  {
    VertexProperty &vertex = m_graph[*vi];
    ModelItem *item = vertex.item;
    if (!item)
      continue;
    Q_ASSERT(item);
    vertex.vId  = item->m_vertex;
    vertex.name = item->data(Qt::DisplayRole).toString().toStdString();
    vertex.args = item->serialize().toStdString();
    switch (item->type())
    {
      case ModelItem::SAMPLE:
        vertex.shape = TRAPEZIUM;
        break;
      case ModelItem::CHANNEL:
	vertex.shape = BOX;
	break;
      case ModelItem::SEGMENTATION:
	vertex.shape = ELLIPSE;
	break;
      case ModelItem::FILTER:
      {
	Filter *filter = dynamic_cast<Filter *>(item);
	Q_ASSERT(filter);
	vertex.shape = INVTRIANGLE;
        break;
      }
    default:
        Q_ASSERT(false);
    }
  }
}

//-----------------------------------------------------------------------------
RelationshipGraph::VertexDescriptor RelationshipGraph::vertex(ModelItem* item)
{
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
void RelationshipGraph::addRelation(ModelItem* ancestor,
				    ModelItem* successor,
				    const QString description)
{
  EdgeProperty p;
  p.relationship = description.toStdString();
  boost::add_edge(vertex(ancestor), vertex(successor), p, m_graph);
}

//-----------------------------------------------------------------------------
void RelationshipGraph::connect(const QString& ancestor, ModelItem* successor, const QString description)
{
  VertexIterator vi, vi_end;
  for(boost::tie(vi, vi_end) = boost::vertices(m_graph); vi != vi_end; vi++)
  {
    Q_ASSERT(false);
//     if(m_graph[*vi].item->getArgument("Id") == id)
//     {
//       break;
//     }
  }
  addRelation(m_graph[*vi].item, successor, description);
}

//-----------------------------------------------------------------------------
Edges RelationshipGraph::edges(const QString filter)
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
Vertices RelationshipGraph::vertices()
{
  Vertices result;

  VertexIterator vi, vi_end;
  for(boost::tie(vi, vi_end) = boost::vertices(m_graph); vi != vi_end; vi++)
  {
//     qDebug() << *vi << m_graph[*vi].name.c_str() << m_graph[*vi].args.c_str();
    Q_ASSERT(m_graph[*vi].vId == *vi);
    result << m_graph[*vi];
  }

  return result;
}

//-----------------------------------------------------------------------------
Vertices RelationshipGraph::ancestors(RelationshipGraph::VertexId v, const QString filter)
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
Vertices RelationshipGraph::succesors(RelationshipGraph::VertexId v, const QString filter)
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
void RelationshipGraph::setItem(RelationshipGraph::VertexId v, ModelItem* item)
{
  m_graph[v].item = item;
}


//-----------------------------------------------------------------------------
QString RelationshipGraph::name(RelationshipGraph::VertexId v) const
{
  return QString(m_graph[v].name.c_str());
}

//-----------------------------------------------------------------------------
ModelItem::ItemType RelationshipGraph::type(const VertexProperty v)
{
  if (v.item)
    return v.item->type();
  else if (v.shape == TRAPEZIUM)
    return ModelItem::SAMPLE;
  else if (v.shape == BOX)
    return ModelItem::CHANNEL;
  else if (v.shape == ELLIPSE)
    return ModelItem::SEGMENTATION;
  else if (v.shape == INVTRIANGLE)
    return ModelItem::FILTER;

  Q_ASSERT(false);
  return ModelItem::TAXONOMY;
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

  dp.property("node_id", boost::get(&VertexProperty::vId, m_graph));
  dp.property("label", boost::get(&VertexProperty::name, m_graph));
  dp.property("shape", boost::get(&VertexProperty::shape, m_graph));
  dp.property("args",  boost::get(&VertexProperty::args, m_graph));
  dp.property("label", boost::get(&EdgeProperty::relationship, m_graph));

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

      dp.property("node_id"    , boost::get(boost::vertex_index, m_graph));
      dp.property("label", boost::get(&VertexProperty::name, m_graph));
      dp.property("shape",       boost::get(&VertexProperty::shape, m_graph));
      dp.property("args",        boost::get(&VertexProperty::args, m_graph));
      dp.property("label",   boost::get(&EdgeProperty::relationship, m_graph));
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
  this->updateVertexInformation();
  switch (format)
  {
    case BOOST:
      stream << boost::write(m_graph) << std::endl;
//       std::cout << boost::write(m_graph) << std::endl;
      break;
    case GRAPHVIZ:
    {
      boost::dynamic_properties dp;

      dp.property("node_id"    , boost::get(boost::vertex_index, m_graph));
      dp.property("label", boost::get(&VertexProperty::name, m_graph));
      dp.property("shape",       boost::get(&VertexProperty::shape, m_graph));
      dp.property("args",        boost::get(&VertexProperty::args, m_graph));
      dp.property("label",   boost::get(&EdgeProperty::relationship, m_graph));
      boost::write_graphviz_dp(stream, m_graph, dp);

      break;
    }
    default:
      qWarning("Format Unkown");
  };

  
}


/*

std::vector< TraceNode* > RelationshipGraph::inputs(const TraceNode* node)
{
  std::vector<TraceNode *> inputNodes;
  return inputNodes;
}

std::vector< TraceNode* > RelationshipGraph::outputs(const TraceNode* node)
{
  std::vector<TraceNode *> result;
  // Node prooperty map
  property_map<Graph, TraceNode * VertexProperty::*>::type nodeMap =
    get(&VertexProperty::node,m_graph);
  // Find targets of output edges
    qDebug() << out_degree(node->localId,m_graph);
  EdgeIteratorRange edges = out_edges(node->localId,m_graph);
  for (OutEdgeIter e = edges.first; e != edges.second; e++)
  {
    VertexId v = target(*e,m_graph);
    result.push_back(nodeMap[v]);
  }
  return result;
}
*/


