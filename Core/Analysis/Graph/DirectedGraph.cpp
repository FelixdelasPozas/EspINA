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
#include <Core/Analysis/Extensions/ExtensionProvider.h>

#include <iostream>
#undef foreach // Due to Qt-Boost incompatibility
#include <boost/graph/graphviz.hpp>
#include <boost/algorithm/string.hpp>

#include <QDebug>

using namespace boost;
using namespace EspINA;

//-----------------------------------------------------------------------------
DirectedGraph::DirectedGraph()
: m_graph(0)
{
}

//-----------------------------------------------------------------------------
void DirectedGraph::add(Vertex vertex)
{
  if (vertex == nullptr) throw (Null_Item_Exception());

  if (contains(vertex)) throw (Existing_Item_Exception());

  VertexDescriptor vd = add_vertex(m_graph);

  m_graph[vd] = vertex;
}

//-----------------------------------------------------------------------------
void DirectedGraph::remove(Vertex vertex)
{
  VertexDescriptor vd = descriptor(vertex);

  clear_vertex (vd, m_graph);
  remove_vertex(vd, m_graph);
}

// //-----------------------------------------------------------------------------
// DirectedGraph::Vertex DirectedGraph::vertex(Vertex item) const
// {
//   //   qDebug() << "Previous id" << item->m_vertex;
//   Vertex v;
// 
//   VertexIterator vi, vi_end;
//   boost::tie(vi, vi_end) = boost::vertices(m_graph);
// //   while(!v.item && vi != vi_end)
// //   {
// //     VertexDescriptor vd = *vi;
// // 
// //     if(m_graph[vd].item.get() == item.get())
// //     {
// //       m_graph[vd].descriptor = vd;
// //       v = m_graph[vd];
// //     }
// // 
// //     ++vi;
// //   }
// 
//   return v;
// }

//-----------------------------------------------------------------------------
void DirectedGraph::addRelation(Vertex ancestor,
                                Vertex successor,
                                const QString&   description)
{
  if (ancestor == nullptr || successor == nullptr) throw (Null_Item_Exception());

  EdgeProperty p;
  p.relationship = description.toStdString();

  if (ancestor == nullptr || successor == nullptr) throw (Item_Not_Found_Exception());

  VertexDescriptor avd = descriptor(ancestor);
  VertexDescriptor svd = descriptor(successor);

  try
  {
   findRelation(avd, svd, description);
   throw (Existing_Relation_Exception());
  } catch (Relation_Not_Found_Exception e) {
   add_edge(avd, svd, p, m_graph);
  }
}

//-----------------------------------------------------------------------------
void DirectedGraph::removeRelation(Vertex   ancestor,
                                       Vertex   successor,
                                       const QString &description)
{
  VertexDescriptor avd = descriptor(ancestor);
  VertexDescriptor svd = descriptor(successor);

  OutEdgeIterator edge = findRelation(avd, svd, description);
  remove_edge(edge, m_graph);
}

//-----------------------------------------------------------------------------
bool DirectedGraph::contains(Vertex vertex)
{
  VertexIterator vi, vi_end;
  for(tie(vi, vi_end) = boost::vertices(m_graph); vi != vi_end; ++vi)
  {
    if (m_graph[*vi] == vertex) return true; 
  }

  return false;
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
DirectedGraph::Edges DirectedGraph::inEdges(Vertex vertex, const QString& filter)
{
  Edges result;

  VertexDescriptor vd = descriptor(vertex);

  InEdgeIterator ei, ei_end;
  for(tie(ei, ei_end) = in_edges(vd, m_graph); ei != ei_end; ei++)
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
DirectedGraph::Edges DirectedGraph::outEdges(Vertex vertex, const QString& filter)
{
  Edges result;

  VertexDescriptor vd = descriptor(vertex);

  OutEdgeIterator ei, ei_end;
  for(tie(ei, ei_end) = out_edges(vd, m_graph); ei != ei_end; ei++)
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
DirectedGraph::Edges DirectedGraph::edges(Vertex vertex, const QString& filter)
{
  Edges result;

  result << inEdges (vertex, filter);
  result << outEdges(vertex, filter);

  return result;
}

//-----------------------------------------------------------------------------
void DirectedGraph::removeEdges(Vertex vertex)
{
  OutEdgeIterator oei, oei_end;
  VertexDescriptor vd = descriptor(vertex);
  boost::tie(oei, oei_end) = boost::out_edges(vd, m_graph); 
  while(oei != oei_end)
  {
    boost::remove_edge(oei, m_graph);
    boost::tie(oei, oei_end) = boost::out_edges(vd, m_graph); 
  }

  Vertices ancestorList = ancestors(vertex);
  for (int i = 0; i < ancestorList.size(); i++)
  {
    VertexDescriptor avd = descriptor(ancestorList[i]);
    boost::tie(oei, oei_end) = boost::out_edges(avd, m_graph); 
    while(oei != oei_end)
    {
      if (target(*oei, m_graph) == vd)
      {
        boost::remove_edge(oei, m_graph);
        boost::tie(oei, oei_end) = boost::out_edges(avd, m_graph); 
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
    result << m_graph[*vi];
  }

  return result;
}

//-----------------------------------------------------------------------------
DirectedGraph::Vertices DirectedGraph::ancestors(Vertex vertex, const QString& filter) const
{
  return ancestors(vertex.get(), filter);
}

//-----------------------------------------------------------------------------
DirectedGraph::Vertices DirectedGraph::ancestors(VertexPtr vertex, const QString& filter) const
{
  Vertices result;
  InEdgeIterator iei, iei_end;

  VertexDescriptor vd = descriptor(vertex);
  //   qDebug() << "Ancestors of:" << m_graph[v].name.c_str();
  for(boost::tie(iei, iei_end) = boost::in_edges(vd, m_graph); iei != iei_end; iei++)
  {
    //     qDebug() << "\t" << source(*iei, m_graph) << m_graph[source(*iei,m_graph)].name.c_str();
    if (filter.isEmpty() || m_graph[*iei].relationship == filter.toStdString())
    {
      //       qDebug() << "Pass Filter:"  << m_graph[source(*iei, m_graph)].descriptor << m_graph[source(*iei, m_graph)].name.c_str();
      VertexDescriptor avd = source(*iei, m_graph);
      result << m_graph[avd];
    }
  }

  return result;
}


//-----------------------------------------------------------------------------
DirectedGraph::Vertices DirectedGraph::succesors(Vertex vertex, const QString& filter) const
{
  return succesors(vertex.get(), filter);
}

//-----------------------------------------------------------------------------
DirectedGraph::Vertices DirectedGraph::succesors(VertexPtr vertex, const QString& filter) const
{
  Vertices result;
  OutEdgeIterator oei, oei_end;

  VertexDescriptor vd = descriptor(vertex);
  //   qDebug() << "Successors of:" << m_graph[v].name.c_str();
  for(boost::tie(oei, oei_end) = boost::out_edges(vd, m_graph); oei != oei_end; oei++)
  {
    //     qDebug() << "\t" << m_graph[target(*oei,m_graph)].name.c_str();
    if (filter.isEmpty() || m_graph[*oei].relationship == filter.toStdString())
    {
      VertexDescriptor svd = target(*oei, m_graph);
      result << m_graph[svd];
    }
  }

  return result;
}

//-----------------------------------------------------------------------------
DirectedGraph::Vertex DirectedGraph::vertex(VertexDescriptor descriptor) const
{
  return m_graph[descriptor];
}

//-----------------------------------------------------------------------------
DirectedGraph::VertexDescriptor DirectedGraph::descriptor(Vertex vertex) const
{
  return descriptor(vertex.get());
}

//-----------------------------------------------------------------------------
DirectedGraph::VertexDescriptor DirectedGraph::descriptor(VertexPtr vertex) const
{
  VertexIterator vi, vi_end;
  for(tie(vi, vi_end) = boost::vertices(m_graph); vi != vi_end; ++vi)
  {
    if (m_graph[*vi].get() == vertex) return *vi;
  }

  throw (Item_Not_Found_Exception());
}


//-----------------------------------------------------------------------------
DirectedGraph::OutEdgeIterator DirectedGraph::findRelation(const VertexDescriptor source,
                                                           const VertexDescriptor destination,
                                                           const QString&         relation) const
{
  OutEdgeIterator ei, oei_end;

  for(tie(ei, oei_end) = out_edges(source, m_graph); ei != oei_end; ++ei)
  {
    if (target(*ei, m_graph) == destination)
      if(m_graph[*ei].relationship == relation.toStdString())
        return ei;
  }

  throw (Relation_Not_Found_Exception());
  return oei_end;
}
