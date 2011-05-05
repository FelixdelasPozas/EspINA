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

#include "processingTrace.h"

#include "graphHelper.h"
#include "espina.h"

#include <iostream>
#include <boost/graph/graphviz.hpp>

//Debug
#include <QDebug>
#include <QStringList>
#include <pqApplicationCore.h>
#include <pqObjectBuilder.h>
#include <pqLoadDataReaction.h>
#include <cachedObjectBuilder.h>

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
void ProcessingTrace::readTrace(std::istream& content)
{
  m_trace.clear(); // Reset the old trace
  Graph schema;
  boost::dynamic_properties dp;
//   boost::property_map<Graph, boost::vertex_index1_t>::type vIndex
//     = boost::get(boost::vertex_index, m_trace);
  dp.property("node_id", boost::get(boost::vertex_index1, schema));
//   boost::property_map<Graph, std::string VertexProperty::*>::type vString;
//   vString = boost::get(&VertexProperties::labelName, m_trace);
  dp.property("label", boost::get(&VertexProperties::labelName, schema));
  //boost::property_map<Graph, std::string VertexProperty::*>::type vShape
  //vString = boost::get(&VertexProperties::shape, m_trace);
  dp.property("shape", boost::get(&VertexProperties::shape, schema));
  //boost::property_map<Graph, std::string VertexProperty::*>::type vShape
  //vString = boost::get(&VertexProperties::args, m_trace);
  dp.property("args", boost::get(&VertexProperties::args, schema));
  
  dp.property("label", boost::get(boost::edge_name, schema));

  boost::read_graphviz( content, schema, dp);

  // Retrieve vertex porperties
  pqApplicationCore* core = pqApplicationCore::instance();
  boost::property_map<Graph, std::string VertexProperty::*>::type vLabel
    = boost::get(&VertexProperty::labelName, schema);
  boost::property_map<Graph, std::string VertexProperty::*>::type vArgs
    = boost::get(&VertexProperty::args, schema);
  boost::property_map<Graph, std::string VertexProperty::*>::type vShape
    = boost::get(&VertexProperty::shape, schema);
  
  // recorrer el grafo en anchura e ir cargando los plugins
  // Initialize PipeLine with rootId
  // while pipeline not empty
  //    retieve their neighbors of the first element in the pipeline(Adacent_vertices) ¡¡Test!!
  //    if pass all the dependecies  and it is a Filter type
  //        build it
  //    else
  //        insert into the pipeline

  QMap< VertexId, QList< VertexId > > parentsMap = predecessors<Graph, VertexId>(schema);
  QList<VertexId> verticesToProcess(rootVertices<Graph, VertexId>(schema)); // The nodes not processed
  QList<VertexId> processedVertices; 
  
  while( !verticesToProcess.empty() )
  {
    VertexId vertexId = verticesToProcess.first();
    qDebug() << "ProcessingTrace: Processing the vertex with id: " << vertexId;
    // ***** Process the vertex *****
    // Retrieve all the parents by giving a VertexId TODO
    bool parentsProcessed = true;
    foreach(VertexId v, parentsMap.value( vertexId ))
    {
      if( ! processedVertices.contains( v ) )
      {
        parentsProcessed = false;
        break;
      }
    }
    if( parentsProcessed )
    {
      QString label(vLabel[vertexId].c_str());
      // Is a stack //TODO be more explicit
      if( vShape[vertexId].compare("ellipse") == 0 && label.startsWith('/') )
      {
        qDebug() << "ProcessingTrace: Loading the Stack " << label;
        //pqPipelineSource* proxy = pqLoadDataReaction::loadData(QStringList(label));
        //assert(NULL == CachedObjectBuilder::instance()->registerLoadedStack(label, proxy));
        EspinaProxy* proxy = CachedObjectBuilder::instance()->createStack(label);
        assert(proxy);
        EspINA::instance()->addSample(proxy, 0, label);
      } // A filter
      else if( vShape[vertexId].compare("box") == 0 )
      { // A filter that can be processed
        qDebug() << "ProcessingTrace: Creating the filter " << label;
        //QStringList filterInfo = QString(vLabel[vertexId].c_str()).split("::");
        //assert(filterInfo.size() == 2);
        QString rawArgs( vArgs[vertexId].c_str() );
        NodeParamList args = parseArgs( rawArgs );
       // if( filterInfo.at(1) == "SeedGrowSegmentationFilter")
        QString pluginName(vLabel[vertexId].c_str());
        EspinaPlugin* plugin = m_availablePlugins.value(pluginName, NULL);
        if( plugin )
          m_availablePlugins.value(pluginName)->LoadAnalisys(args);
        else
          qDebug() << "ProcessingTrace: the filter is not registered";

//        core->getObjectBuilder()->createFilter(qstrl.at(0), qstrl.at(1),
 //                                              pqPipelineSource  );
  //
        //core->getObjectBuilder()->createSource()
      } // A segmentation
      else {
        qDebug() << "ProcessingTrace: A segmentation ... ignored";
      //  localPipe.push_back(*vi);
      // check if is a filter and all of its dependecies exist
      }
      processedVertices.append( vertexId );
      verticesToProcess.pop_front(); // Remove the processed element
        
      // ***** Retrieve the adjacet vertices *****
      boost::graph_traits<Graph>::adjacency_iterator vi, vi_end;
      for(boost::tie(vi, vi_end) = boost::adjacent_vertices(vertexId, schema);
          vi != vi_end; vi++)
      {
        verticesToProcess.push_front(*vi);
      }
      
    }
    else // The node could not be processed. Swap it with the next one
    {
      verticesToProcess.swap(0,1);
    }   
  }
}


//-----------------------------------------------------------------------------
void ProcessingTrace::readTrace(QTextStream& stream)
{
  m_trace.clear(); // Reset the old trace
  Graph schema;
  boost::dynamic_properties dp;
//   boost::property_map<Graph, boost::vertex_index1_t>::type vIndex
//     = boost::get(boost::vertex_index, m_trace);
  dp.property("node_id", boost::get(boost::vertex_index1, schema));
//   boost::property_map<Graph, std::string VertexProperty::*>::type vString;
//   vString = boost::get(&VertexProperties::labelName, m_trace);
  dp.property("label", boost::get(&VertexProperties::labelName, schema));
  //boost::property_map<Graph, std::string VertexProperty::*>::type vShape
  //vString = boost::get(&VertexProperties::shape, m_trace);
  dp.property("shape", boost::get(&VertexProperties::shape, schema));
  //boost::property_map<Graph, std::string VertexProperty::*>::type vShape
  //vString = boost::get(&VertexProperties::args, m_trace);
  dp.property("args", boost::get(&VertexProperties::args, schema));

  dp.property("label", boost::get(boost::edge_name, schema));
  //qDebug() <<  stream.string();
  boost::read_graphviz( stream.string()->toStdString(), schema, dp);

  // Retrieve vertex porperties
  pqApplicationCore* core = pqApplicationCore::instance();
  boost::property_map<Graph, std::string VertexProperty::*>::type vLabel
    = boost::get(&VertexProperty::labelName, schema);
  boost::property_map<Graph, std::string VertexProperty::*>::type vArgs
    = boost::get(&VertexProperty::args, schema);
  boost::property_map<Graph, std::string VertexProperty::*>::type vShape
    = boost::get(&VertexProperty::shape, schema);

  // recorrer el grafo en anchura e ir cargando los plugins
  // Initialize PipeLine with rootId
  // while pipeline not empty
  //    retieve their neighbors of the first element in the pipeline(Adacent_vertices) ¡¡Test!!
  //    if pass all the dependecies  and it is a Filter type
  //        build it
  //    else
  //        insert into the pipeline

  QMap< VertexId, QList< VertexId > > parentsMap = predecessors<Graph, VertexId>(schema);
  QList<VertexId> verticesToProcess(rootVertices<Graph, VertexId>(schema)); // The nodes not processed
  QList<VertexId> processedVertices;

  while( !verticesToProcess.empty() )
  {
    VertexId vertexId = verticesToProcess.first();
    qDebug() << "ProcessingTrace: Processing the vertex with id: " << vertexId;
    // ***** Process the vertex *****
    // Retrieve all the parents by giving a VertexId TODO
    bool parentsProcessed = true;
    foreach(VertexId v, parentsMap.value( vertexId ))
    {
      if( ! processedVertices.contains( v ) )
      {
        parentsProcessed = false;
        break;
      }
    }
    if( parentsProcessed )
    {
      QString label(vLabel[vertexId].c_str());
      // Is a stack //TODO be more explicit
      if( vShape[vertexId].compare("ellipse") == 0 && label.startsWith('/') )
      {
        qDebug() << "ProcessingTrace: Loading the Stack " << label;
        //pqPipelineSource* proxy = pqLoadDataReaction::loadData(QStringList(label));
        //assert(NULL == CachedObjectBuilder::instance()->registerLoadedStack(label, proxy));
        EspinaProxy* proxy = CachedObjectBuilder::instance()->createStack(label);
        assert(proxy);
        EspINA::instance()->addSample(proxy, 0, label);
      } // A filter
      else if( vShape[vertexId].compare("box") == 0 )
      { // A filter that can be processed
        qDebug() << "ProcessingTrace: Creating the filter " << label;
        //QStringList filterInfo = QString(vLabel[vertexId].c_str()).split("::");
        //assert(filterInfo.size() == 2);
        QString rawArgs( vArgs[vertexId].c_str() );
        NodeParamList args = parseArgs( rawArgs );
       // if( filterInfo.at(1) == "SeedGrowSegmentationFilter")
        
        EspinaPlugin* plugin = m_availablePlugins.value(label, NULL);
        if( plugin )
          m_availablePlugins.value(label)->LoadAnalisys(args);
        else
          qDebug() << "ProcessingTrace: the filter is not registered";

//        core->getObjectBuilder()->createFilter(qstrl.at(0), qstrl.at(1),
 //                                              pqPipelineSource  );
  //
        //core->getObjectBuilder()->createSource()
      } // A segmentation
      else {
        qDebug() << "ProcessingTrace: A segmentation ... ignored";
      //  localPipe.push_back(*vi);
      // check if is a filter and all of its dependecies exist
      }
      processedVertices.append( vertexId );
      verticesToProcess.pop_front(); // Remove the processed element

      // ***** Retrieve the adjacet vertices *****
      boost::graph_traits<Graph>::adjacency_iterator vi, vi_end;
      for(boost::tie(vi, vi_end) = boost::adjacent_vertices(vertexId, schema);
          vi != vi_end; vi++)
      {
        verticesToProcess.push_front(*vi);
      }

    }
    else // The node could not be processed. Swap it with the next one
    {
      verticesToProcess.swap(0,1);
    }
  }
}


//-----------------------------------------------------------------------------
void ProcessingTrace::registerPlugin(QString& groupName,
                                     QString& filterName,
                                     EspinaPlugin* filter)
{
  QString key(groupName);
  key.append("::").append(filterName);
  assert( m_availablePlugins.contains(key) == false );
  m_availablePlugins.insert(key, filter);
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


