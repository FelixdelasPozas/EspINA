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
#include <QStringList>
#include <pqApplicationCore.h>
#include <pqObjectBuilder.h>

using namespace boost;

ProcessingTrace* ProcessingTrace::m_instnace(NULL);

//-----------------------------------------------------------------------------
ProcessingTrace* ProcessingTrace::instance()
{
  if( !m_instnace )
    m_instnace = new ProcessingTrace();
  return m_instnace;
}

//-----------------------------------------------------------------------------
ProcessingTrace::ProcessingTrace()
: m_trace(0)
{
  //m_trace[0].trace_name = "Espina";
}

//-----------------------------------------------------------------------------
ProcessingTrace::ProcessingTrace(const QString& name)
: m_trace(0)
{
 // m_trace.m_property.owner = name;
}

//-----------------------------------------------------------------------------
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

//-----------------------------------------------------------------------------
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

//-----------------------------------------------------------------------------
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
  qDebug() << "After read the file I should build the pipeline ...";
  // Take the nodes that do not have input arrows
  // For i in them:

  QList<VertexId> rootIds = rootVertices();
  qDebug() << "The root ids" << rootIds;
  

  pqApplicationCore* core = pqApplicationCore::instance();

  //Load Stack
  /*
  if (boost::filesystem::exists(index.toStdString().c_str()))
    proxy = ob->createReader("sources","MetaImageReader",file,server);
  */
/*
  QStringList qstrl;
  NodeParamList args;
  // Retrieve vertex porperties
  boost::property_map<Graph, std::string VertexProperty::*>::type vLabel
    = boost::get(&VertexProperty::labelName, m_trace);
  boost::property_map<Graph, std::string VertexProperty::*>::type vArgs
    = boost::get(&VertexProperty::args, m_trace);
  boost::property_map<Graph, std::string VertexProperty::*>::type vShape
    = boost::get(&VertexProperty::shape, m_trace);

  // Iter upon vertices
  boost::graph_traits<Graph>::vertex_iterator vi, vi_end;
  for( boost::tie(vi, vi_end) = boost::vertices(m_trace); vi != vi_end; vi++)
  {
    // If it is a filter, it must parse the args
    if( vShape[*vi].compare("box") == 0 )
    {
      qstrl = QString(vLabel[*vi].c_str()).split("::");
      assert(qstrl.size() == 2);
      QString rawArgs( vArgs[*vi].c_str() );
      args = parseArgs( rawArgs );
      
      core->getObjectBuilder()->createFilter(qstrl.at(0), qstrl.at(1),
                                             );
      
      /*
      foreach( NodeParam param, args){
        qDebug() << "\t" << param.first << " = " << param.second;
      }
      *
    } // A stack or a product
    else if( vShape[*vi].compare("ellipse") == 0 )
    {
      if( vLabel[*vi] != "Product" )
        qDebug() << "\tPosible stack!";
    }
  }

  
  /**/

 
  
}

//-----------------------------------------------------------------------------
NodeParamList ProcessingTrace::parseArgs( QString& raw )
{
  NodeParamList res;
  QStringList argList;
  foreach(QString arg, raw.split(";"))
  {
    argList = arg.split(":");
    res.push_back(NodeParam(argList[0], argList[1]));
  }
  return res;
}

//-----------------------------------------------------------------------------
/// Visit nodes by edges and return the root vertex id
QList<ProcessingTrace::VertexId> ProcessingTrace::rootVertices()
{
    VertexId v;
    QList<VertexId> discardedVertices, posibleRootVertices;

    boost::graph_traits<Graph>::edge_iterator ei, ei_end;
    for( boost::tie(ei, ei_end) = boost::edges(m_trace); ei != ei_end; ei++)
    {
      // std::cout << *ei;
      v = boost::source(*ei, m_trace);
      //std::cout << " v" << v;
      if( !discardedVertices.contains(v) && !posibleRootVertices.contains(v)){
        // std::cout << " a posible root vertex ";
        posibleRootVertices.push_back(v);
      }
      // std::cout << std::endl;
      v = boost::target(*ei, m_trace);
      if( posibleRootVertices.contains(v) ){
        // std::cout << "v" << v << " deleted from posibleRootVertices "<< std::endl;
        posibleRootVertices.removeOne(v);
      }
      discardedVertices.push_back(v);
      /*
      qDebug() << "Discarded" << discardedVertices;
      qDebug() << "Posible Root" << posibleRootVertices;
      */
    }
    assert(posibleRootVertices.size() >= 1 );//? posibleRootVertices.at(0) : -1;
    return posibleRootVertices;

}

//-----------------------------------------------------------------------------
void ProcessingTrace::print( std::ostream& out, ProcessingTrace::printFormat format)
{
  if( format == graphviz )
  {
    boost::dynamic_properties dp;

    dp.property("node_id", boost::get(boost::vertex_index, m_trace));
    dp.property("label", boost::get(&VertexProperties::labelName, m_trace));
    dp.property("shape", boost::get(&VertexProperties::shape, m_trace));
    dp.property("args", boost::get(&VertexProperties::args, m_trace));
    dp.property("label", boost::get(boost::edge_name, m_trace));

    boost::write_graphviz( out, m_trace, dp);
  }
  else if( format == debug)
  {
    QStringList qstrl;
    NodeParamList args;
    // Retrieve vertex porperties
    boost::property_map<Graph, std::string VertexProperty::*>::type vLabel
      = boost::get(&VertexProperty::labelName, m_trace);
    boost::property_map<Graph, std::string VertexProperty::*>::type vArgs
      = boost::get(&VertexProperty::args, m_trace);
    boost::property_map<Graph, std::string VertexProperty::*>::type vShape
      = boost::get(&VertexProperty::shape, m_trace);
    // Iter upon vertices
    boost::graph_traits<Graph>::vertex_iterator vi, vi_end;
    for( boost::tie(vi, vi_end) = boost::vertices(m_trace); vi != vi_end; vi++)
    {
      out << *vi << " - " << vLabel[*vi] << std::endl;
      // If it is a filter, it must parse the args
      if( vShape[*vi].compare("box") == 0 )
      {
        qstrl = QString(vLabel[*vi].c_str()).split("::");
        assert(qstrl.size() == 2);
        QString rawArgs( vArgs[*vi].c_str() );
        args = parseArgs( rawArgs );
        foreach( NodeParam param, args)
        {
          out << "\t" << param.first.toStdString()
              << " = " << param.second.toStdString();
        }
      } // A stack or a product
      else if( vShape[*vi].compare("ellipse") == 0 )
      {
        if( vLabel[*vi] != "Product" )
          out << "\tPosible stack!";
      }
    }
  }
    
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


