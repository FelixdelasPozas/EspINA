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


ProcessingTrace* ProcessingTrace::m_instnace(NULL);

ProcessingTrace* ProcessingTrace::instance()
{
  if( !m_instnace )
    m_instnace = new ProcessingTrace();
  return m_instnace;
}


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
  /*
  property_map<Graph, ITraceNode * VertexProperty::*>::type nodeMap =
    get(&VertexProperty::node,m_trace);
  */
  node->vertexId = v;
  //nodeMap[v] = node;
  m_trace[v].node = node;
  
  QString args;  
  foreach( NodeParam param, node->getArguments())
  {
    if( args.size() )
      args.append(";");
    args.append(param.first + ":" + param.second);
  }
  
  m_trace[v].labelName = node->name.toStdString();
  m_trace[v].args =  args.toStdString();
  if( node->type ) // 0: Product, 1: Filter
    m_trace[v].shape = "box";
  else
    m_trace[v].shape = "ellipse";
  //nodeMap[v]->print();
  //m_trace[v].node->print();
    
  
}

void ProcessingTrace::connect(
  ITraceNode* origin
, ITraceNode* destination
, const std::string& description
)
{
  boost::add_edge(origin->vertexId, destination->vertexId, description, m_trace);
  
  //property_map<Graph, std::string EdgeProperty::*>::type descMap =
  //  get(&EdgeProperty::relationship,m_trace);
  //descMap[e] = description;
  // Get list of vertex_descriptor
  //Find the nodes corresponding the nodes
  // Add a new edge(origin,destination) with 
  // description property
}

void ProcessingTrace::readTrace(std::istream& fileName)
{
  m_trace.clear();
  boost::dynamic_properties dp;
//   boost::property_map<Graph, boost::vertex_index1_t>::type vIndex
//     = boost::get(boost::vertex_index, m_trace);
  dp.property("node_id", boost::get(boost::vertex_index1, m_trace));
//   boost::property_map<Graph, std::string VertexProperty::*>::type vString;
//   vString = boost::get(&VertexProperties::labelName, m_trace);
  dp.property("label", boost::get(&VertexProperties::labelName, m_trace));
  //boost::property_map<Graph, std::string VertexProperty::*>::type vShape
  //vString = boost::get(&VertexProperties::shape, m_trace);
  dp.property("shape", boost::get(&VertexProperties::shape, m_trace));
  //boost::property_map<Graph, std::string VertexProperty::*>::type vShape
  //vString = boost::get(&VertexProperties::args, m_trace);
  dp.property("args", boost::get(&VertexProperties::args, m_trace));
  
  dp.property("label", boost::get(boost::edge_name, m_trace));

  boost::read_graphviz( fileName, m_trace, dp);
  
  //TODO build the pipeline
  qDebug() << "After read the fu*:~ file I should build the pipeline ...";
  
}


void ProcessingTrace::print( std::ostream& out )
{
  boost::dynamic_properties dp;
  
  dp.property("node_id", boost::get(boost::vertex_index, m_trace));
  dp.property("label", boost::get(&VertexProperties::labelName, m_trace));
  dp.property("shape", boost::get(&VertexProperties::shape, m_trace));
  dp.property("args", boost::get(&VertexProperties::args, m_trace));
  dp.property("label", boost::get(boost::edge_name, m_trace));
	      
  boost::write_graphviz( out, m_trace, dp);
  // property_map<Graph, (return type) Class::*)
  //property_map<Graph, std::string VertexProperty::*>::type nameMap =
  //  get(&VertexProperty::name,m_trace);
  //add_edge(0,1,m_trace);
    
  //write_graphviz(std::cout,m_trace,make_label_writer(nameMap));
}

/*
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
*/


