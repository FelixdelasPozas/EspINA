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

// ESPINA
#include "DirectedGraph.h"
#include <Core/Utils/EspinaException.h>

// C++
#include <iostream>

// Boost
#undef foreach // Due to Qt-Boost incompatibility
#include <boost/graph/graphviz.hpp>
#include <boost/algorithm/string.hpp>

// Qt
#include <QDebug>

using namespace boost;
using namespace ESPINA;
using namespace ESPINA::Core::Utils;

//-----------------------------------------------------------------------------
DirectedGraph::DirectedGraph()
: m_graph(0)
{
}

//-----------------------------------------------------------------------------
void DirectedGraph::add(Vertex vertex)
{
  QMutexLocker lock(&m_mutex);

  if (vertex == nullptr)
  {
    auto what    = QObject::tr("Attempt to add null item");
    auto details = QObject::tr("DirectedGraph::add() -> ") + what;

    throw EspinaException(what, details);
  }

  if (contains_implementation(vertex))
  {
    auto what    = QObject::tr("Attempt to add an already existing item");
    auto details = QObject::tr("DirectedGraph::add() -> ") + what;

    throw EspinaException(what, details);
  }

  VertexDescriptor vd = add_vertex(m_graph);

  m_graph[vd] = vertex;
}

//-----------------------------------------------------------------------------
void DirectedGraph::remove(Vertex vertex)
{
  QMutexLocker lock(&m_mutex);

  if (!contains_implementation(vertex))
  {
    auto what    = QObject::tr("Attempt to remove a non existent item");
    auto details = QObject::tr("DirectedGraph::add() -> ") + what;

    throw EspinaException(what, details);
  }

  VertexDescriptor vd = descriptor(vertex);

  clear_vertex (vd, m_graph);
  remove_vertex(vd, m_graph);
}

//-----------------------------------------------------------------------------
void DirectedGraph::addRelation(Vertex ancestor,
                                Vertex successor,
                                const QString& description)
{
  if (ancestor == nullptr || successor == nullptr)
  {
    auto what    = QObject::tr("Invalid relation vertex: %1").arg((ancestor == nullptr ? "ancestor" : "successor"));
    auto details = QObject::tr("DirectedGraph::addRelation() -> Invalid relation vertex: %1").arg((ancestor == nullptr ? "ancestor" : "successor"));

    throw EspinaException(what, details);
  }

  bool alreadyInGraph = false;

  {
    QMutexLocker lock(&m_mutex);

    EdgeProperty p;
    p.relationship = description.toStdString();

    auto avd = descriptor(ancestor);
    auto svd = descriptor(successor);

    OutEdgeIterator oei;

    alreadyInGraph = existsRelation(avd, svd, description);

    if(!alreadyInGraph) add_edge(avd, svd, p, m_graph);
  }

  if(alreadyInGraph)
  {
    auto what    = QObject::tr("Attempt to add and existing relation, ancestor: %1, successor: %2, relation: %3").arg(ancestor->name()).arg(successor->name()).arg(description);
    auto details = QObject::tr("DirectedGraph::addRelation() -> Attempt to add and existing relation, ancestor: %1, successor: %2, relation: %3").arg(ancestor->name()).arg(successor->name()).arg(description);

    throw EspinaException(what, details);
  }
}

//-----------------------------------------------------------------------------
void DirectedGraph::removeRelation(Vertex         ancestor,
                                   Vertex         successor,
                                   const QString &description)
{
  QMutexLocker lock(&m_mutex);

  VertexDescriptor avd = descriptor(ancestor);
  VertexDescriptor svd = descriptor(successor);

  OutEdgeIterator edge = findRelation(avd, svd, description);
  remove_edge(edge, m_graph);
}

//-----------------------------------------------------------------------------
bool DirectedGraph::contains(Vertex vertex) const
{
  QMutexLocker lock(&m_mutex);

  return contains_implementation(vertex);
}

//-----------------------------------------------------------------------------
DirectedGraph::Edges DirectedGraph::edges(const QString &filter) const
{
  QMutexLocker lock(&m_mutex);

  Edges result;

  EdgeIterator ei, ei_end;
  for(boost::tie(ei, ei_end) = boost::edges(m_graph); ei != ei_end; ++ei)
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
DirectedGraph::Edges DirectedGraph::inEdges(Vertex vertex, const QString& filter) const
{
  return inEdges(vertex.get(), filter);
}

//-----------------------------------------------------------------------------
DirectedGraph::Edges DirectedGraph::inEdges(VertexPtr vertex, const QString& filter) const
{
  QMutexLocker lock(&m_mutex);

  return inEdges_implementation(vertex, filter);
}

//-----------------------------------------------------------------------------
DirectedGraph::Edges DirectedGraph::outEdges(Vertex vertex, const QString& filter) const
{
  return outEdges(vertex.get(), filter);
}

//-----------------------------------------------------------------------------
DirectedGraph::Edges DirectedGraph::outEdges(VertexPtr vertex, const QString& filter) const
{
  QMutexLocker lock(&m_mutex);

  return outEdges_implementation(vertex, filter);
}

//-----------------------------------------------------------------------------
DirectedGraph::Edges DirectedGraph::edges(Vertex vertex, const QString& filter) const
{
  QMutexLocker lock(&m_mutex);

  Edges result;

  result << inEdges_implementation(vertex.get(), filter);
  result << outEdges_implementation(vertex.get(), filter);

  return result;
}

//-----------------------------------------------------------------------------
void DirectedGraph::removeEdges(Vertex vertex)
{
  QMutexLocker lock(&m_mutex);

  Vertices ancestorList = ancestors_implementation(vertex.get());

  OutEdgeIterator oei, oei_end;
  VertexDescriptor vd = descriptor(vertex);
  boost::tie(oei, oei_end) = boost::out_edges(vd, m_graph);
  while(oei != oei_end)
  {
    boost::remove_edge(oei, m_graph);
    boost::tie(oei, oei_end) = boost::out_edges(vd, m_graph);
  }

  for (int i = 0; i < ancestorList.size(); ++i)
  {
    VertexDescriptor avd = descriptor(ancestorList[i]);
    boost::tie(oei, oei_end) = boost::out_edges(avd, m_graph);
    while(oei != oei_end)
    {
      if (target(*oei, m_graph) == vd)
      {
        boost::remove_edge(oei, m_graph);
        boost::tie(oei, oei_end) = boost::out_edges(avd, m_graph);
      }
      else
      {
        ++oei;
      }
    }
  }
}

//-----------------------------------------------------------------------------
DirectedGraph::Vertices DirectedGraph::vertices() const
{
  QMutexLocker lock(&m_mutex);

  Vertices result;

  VertexIterator vi, vi_end;
  for(boost::tie(vi, vi_end) = boost::vertices(m_graph); vi != vi_end; ++vi)
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
  QMutexLocker lock(&m_mutex);

  return ancestors_implementation(vertex, filter);
}

//-----------------------------------------------------------------------------
DirectedGraph::Vertices DirectedGraph::successors(Vertex vertex, const QString& filter) const
{
  return successors(vertex.get(), filter);
}

//-----------------------------------------------------------------------------
DirectedGraph::Vertices DirectedGraph::successors(VertexPtr vertex, const QString& filter) const
{
  QMutexLocker lock(&m_mutex);

  Vertices result;
  OutEdgeIterator oei, oei_end;

  VertexDescriptor vd = descriptor(vertex);
  for(boost::tie(oei, oei_end) = boost::out_edges(vd, m_graph); oei != oei_end; ++oei)
  {
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

  auto what    = QObject::tr("Descriptor not found, vertex: %1").arg(vertex->name());
  auto details = QObject::tr("DirectedGraph::descriptor() -> Descriptor not found, vertex: %1").arg(vertex->name());

  throw EspinaException(what, details);

  return DirectedGraph::VertexDescriptor();
}

//-----------------------------------------------------------------------------
bool DirectedGraph::existsRelation(const VertexDescriptor source,
                                   const VertexDescriptor destination,
                                   const QString         &relation) const
{
  OutEdgeIterator ei, oei_end;

  for (tie(ei, oei_end) = out_edges(source, m_graph); ei != oei_end; ++ei)
  {
    if (target(*ei, m_graph) == destination)
    {
      if (m_graph[*ei].relationship == relation.toStdString())
      {
        return true;
      }
    }
  }

  return false;
}

//-----------------------------------------------------------------------------
DirectedGraph::OutEdgeIterator DirectedGraph::findRelation(const VertexDescriptor source,
                                                           const VertexDescriptor destination,
                                                           const QString         &relation) const
{
  OutEdgeIterator ei, oei_end;

  for (tie(ei, oei_end) = out_edges(source, m_graph); ei != oei_end; ++ei)
  {
    if (target(*ei, m_graph) == destination)
    {
      if (m_graph[*ei].relationship == relation.toStdString())
      {
        return ei;
      }
    }
  }

  auto what    = QObject::tr("Relation not found, relation: %1").arg(relation);
  auto details = QObject::tr("DirectedGraph::findRelation() -> Relation not found, relation: %1").arg(relation);

  throw EspinaException(what, details);

  return oei_end;
}

//-----------------------------------------------------------------------------
bool DirectedGraph::contains_implementation(Vertex vertex) const
{
  VertexIterator vi, vi_end;
  for(tie(vi, vi_end) = boost::vertices(m_graph); vi != vi_end; ++vi)
  {
    if (m_graph[*vi] == vertex) return true;
  }

  return false;

}

//-----------------------------------------------------------------------------
void DirectedGraph::clear()
{
  QMutexLocker lock(&m_mutex);

  m_graph.clear();
}

//-----------------------------------------------------------------------------
DirectedGraph::Vertices DirectedGraph::rootAncestors(DirectedGraph::Vertex vertex) const
{
  return rootAncestors(vertex.get());
}

//-----------------------------------------------------------------------------
DirectedGraph::Vertices DirectedGraph::rootAncestors(DirectedGraph::VertexPtr vertex) const
{
  QMutexLocker lock(&m_mutex);

  DirectedGraph::Vertices rootAncestors;

  auto ancestors = ancestors_implementation(vertex);

  while (!ancestors.isEmpty())
  {
    auto ancestor = ancestors.takeFirst();

    auto grandAncestors = ancestors_implementation(ancestor.get());

    if (grandAncestors.isEmpty())
    {
      if (!rootAncestors.contains(ancestor))
      {
        rootAncestors << ancestor;
      }
    }
    else
    {
      ancestors << grandAncestors;
    }
  }


  return rootAncestors;
}

//-----------------------------------------------------------------------------
DirectedGraph::Edges DirectedGraph::inEdges_implementation(VertexPtr vertex, const QString& filter) const
{
  Edges result;

  VertexDescriptor vd = descriptor(vertex);

  InEdgeIterator ei, ei_end;
  for(tie(ei, ei_end) = in_edges(vd, m_graph); ei != ei_end; ++ei)
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
DirectedGraph::Edges DirectedGraph::outEdges_implementation(VertexPtr vertex, const QString& filter) const
{
  Edges result;

  VertexDescriptor vd = descriptor(vertex);

  OutEdgeIterator ei, ei_end;
  for(tie(ei, ei_end) = out_edges(vd, m_graph); ei != ei_end; ++ei)
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
DirectedGraph::Vertices DirectedGraph::ancestors_implementation(VertexPtr vertex, const QString& filter) const
{
  Vertices result;
  InEdgeIterator iei, iei_end;

  VertexDescriptor vd = descriptor(vertex);

  for(boost::tie(iei, iei_end) = boost::in_edges(vd, m_graph); iei != iei_end; ++iei)
  {
    if (filter.isEmpty() || m_graph[*iei].relationship == filter.toStdString())
    {
      VertexDescriptor avd = source(*iei, m_graph);
      result << m_graph[avd];
    }
  }

  return result;
}
