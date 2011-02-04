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

#include "tracing.h"

#include <iostream>
#include <boost/graph/graphviz.hpp>

//Debug
#include <QDebug>

using namespace boost;


ProcessingTrace::ProcessingTrace()
: m_trace(0)
{
  //m_trace[0].trace_name = "Espina";
}

ProcessingTrace::ProcessingTrace(const QString& name)
: m_trace(0)
{
 // m_trace.m_property.owner = name;
}


void ProcessingTrace::addNode(ITraceNode* node)
{
  VertexId v = add_vertex(m_trace);
  property_map<Graph, ITraceNode * VertexProperty::*>::type nodeMap =
    get(&VertexProperty::node,m_trace);
  node->localId = v;
  nodeMap[v] = node;
  nodeMap[v]->print();
}

void ProcessingTrace::connect(
  ITraceNode* origin
, ITraceNode* destination
, const std::string& description
)
{
  add_edge(origin->localId,destination->localId,m_trace);
  //property_map<Graph, std::string EdgeProperty::*>::type descMap =
  //  get(&EdgeProperty::relationship,m_trace);
  //descMap[e] = description;
  // Get list of vertex_descriptor
  //Find the nodes corresponding the nodes
  // Add a new edge(origin,destination) with 
  // description property
}


void ProcessingTrace::print()
{
  // property_map<Graph, (return type) Class::*)
  //property_map<Graph, std::string VertexProperty::*>::type nameMap =
  //  get(&VertexProperty::name,m_trace);
  //add_edge(0,1,m_trace);
    
  //write_graphviz(std::cout,m_trace,make_label_writer(nameMap));
}

void ProcessingTrace::addSubtrace(const ProcessingTrace* subTrace)
{

}

std::vector< ITraceNode* > ProcessingTrace::inputs(const ITraceNode* node)
{
  std::vector<ITraceNode *> inputNodes;
  return inputNodes;
}

std::vector< ITraceNode* > ProcessingTrace::outputs(const ITraceNode* node)
{
  std::vector<ITraceNode *> result;
  // Node prooperty map
  property_map<Graph, ITraceNode * VertexProperty::*>::type nodeMap =
    get(&VertexProperty::node,m_trace);
  // Find targets of output edges
    qDebug() << out_degree(node->localId,m_trace);
  EdgeIteratorRange edges = out_edges(node->localId,m_trace);
  for (OutEdgeIter e = edges.first; e != edges.second; e++)
  {
    VertexId v = target(*e,m_trace);
    result.push_back(nodeMap[v]);
  }
  return result;
}



