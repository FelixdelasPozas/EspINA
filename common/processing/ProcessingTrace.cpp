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

#include "espina.h"
#include "sample.h"
#include "segmentation.h"

#include "graphHelper.h"
#include "EspinaPlugin.h"

#include <iostream>
#include <boost/graph/graphviz.hpp>

//Debug
#include <QDebug>
#include <QStringList>
#include <pqApplicationCore.h>
#include <pqObjectBuilder.h>
#include <pqLoadDataReaction.h>
#include <cachedObjectBuilder.h>
#include "espINAFactory.h"
#include "labelMapExtension.h"

#include <QSettings>
#include <QFileDialog>
#include "EspinaPluginManager.h"

using namespace boost;



//-----------------------------------------------------------------------------
ITraceNode::Arguments ITraceNode::parseArgs(QString& raw)
{
  ITraceNode::Arguments res;
  QStringList argList;
  QString name, value, buffer;
  int balanceo = 0;

  foreach(QChar c, raw)
  {
    if( c == '=' && balanceo == 0)
    {
      name = buffer;
      buffer = "";
    }    
    else if( c == '[')
    {
      if(balanceo > 0)
        buffer.append(c);
      balanceo++;
    }
    else if( c== ']')
    {
      balanceo--;
      if(balanceo > 0)
        buffer.append(c);
    }
    else if( c == ';' && balanceo == 0)
    {
      value = buffer;
      buffer = "";
      res.insert(name, value);
    }
    else
    {
      buffer.append(c);
    }      
  }

  return res;
}


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
  //node->vertexId = v;
  m_trace[v].node = node;
}


//-----------------------------------------------------------------------------
void ProcessingTrace::readNodes()
{
  boost::graph_traits<Graph>::vertex_iterator vi, vi_end;
  for(boost::tie(vi, vi_end) = boost::vertices(m_trace); vi != vi_end; vi++)
  {
    ITraceNode* node = m_trace[*vi].node;
    assert(node);
    m_trace[*vi].labelName = node->label().toStdString();
    m_trace[*vi].args =  node->getArguments().toStdString();
    switch (node->type)
    {
    case (ITraceNode::FILTER):
        m_trace[*vi].shape = "box";
        break;
    case (ITraceNode::PRODUCT):
        m_trace[*vi].shape = "ellipse";
        break;
    default:
        assert(false);
    }
  }
}

//-----------------------------------------------------------------------------
graph_traits< ProcessingTrace::Graph >::vertex_descriptor
ProcessingTrace::vertexIndex(ITraceNode* arg1)
{
  boost::graph_traits<Graph>::vertex_iterator vi, vi_end;
  for(boost::tie(vi, vi_end) = boost::vertices(m_trace); vi != vi_end; vi++)
  {
    if( m_trace[*vi].node == arg1 )
      return *vi;
  }
  
  assert(false);
}

//-----------------------------------------------------------------------------
void ProcessingTrace::connect(
  ITraceNode* origin
, ITraceNode* destination
, const std::string& description
)
{
  //boost::add_edge(origin->vertexId, destination->vertexId, description, m_trace);
  boost::add_edge(vertexIndex(origin), vertexIndex(destination), description, m_trace);
  
  //property_map<Graph, std::string EdgeProperty::*>::type descMap =
  //  get(&EdgeProperty::relationship,m_trace);
  //descMap[e] = description;
  // Get list of vertex_descriptor
  //Find the nodes corresponding the nodes
  // Add a new edge(origin,destination) with 
  // description property
}

//-----------------------------------------------------------------------------
void ProcessingTrace::connect(const QString& id,
                              ITraceNode* destination,
                              const std::string& description)
{
  boost::graph_traits<Graph>::vertex_iterator vi, vi_end;
  for(boost::tie(vi, vi_end) = boost::vertices(m_trace); vi != vi_end; vi++)
  {
    if(m_trace[*vi].node->getArgument("Id") == id)
    {
      break;
    }
  }
  connect(m_trace[*vi].node, destination, description);
}

//-----------------------------------------------------------------------------
void ProcessingTrace::removeNode(ITraceNode* node)
{
  EspinaFilter *parent = NULL;
  graph_traits< ProcessingTrace::Graph >::vertex_descriptor indexNode = vertexIndex(node);
  clear_vertex(indexNode, m_trace);
  remove_vertex(indexNode, m_trace);
  Segmentation *seg = dynamic_cast<Segmentation *>(node);
  if( seg )
  {
    assert( node->type == ITraceNode::PRODUCT );
    parent = seg->parent();
    parent->removeProduct(seg);
    if (parent->numProducts() > 0)
    {
      parent = NULL;
    }
  }
  delete node;
  // The parent must be deleted after childs
  if( parent )
    removeNode(dynamic_cast<ITraceNode*>(parent));
  
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
  
  Sample *newSample;
  
  int lastId = 0;

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
      QString rawArgs( vArgs[vertexId].c_str() );
      ITraceNode::Arguments args = ITraceNode::parseArgs( rawArgs );
      // Is a stack //TODO be more explicit
      if( vShape[vertexId].compare("ellipse") == 0 && label.contains(".") ) //Samples contains extension's dot
      {
        qDebug() << "ProcessingTrace: Loading the Stack " << label;
	//NOTE: I call load reaction data to be sure it is also in the server
	//TODO: review and clean
	// First we try to find the sample in the current directory
	QString path = label.section('/',0,-2);
	QString sampleId = label.section('/', -1);
	QString sampleFile = label;
        pqPipelineSource* proxy = pqLoadDataReaction::loadData(QStringList(sampleFile));
	if (!proxy) // Try to find the sample in configuration directory
	{
	  QDir workingDirectory = Cache::instance()->workingDirectory();
	  workingDirectory.cdUp();
	  path = workingDirectory.absolutePath();
	  sampleFile = path + '/' + sampleId;
	  proxy = pqLoadDataReaction::loadData(QStringList(sampleFile));
	  if (!proxy)
	  {
	    QSettings settings;
	    path = settings.value("samplePath").toString();;
	    sampleFile = path + '/' + sampleId;
	    proxy = pqLoadDataReaction::loadData(QStringList(sampleFile));
	    if (!proxy)
	    {      
	      sampleFile =  QFileDialog::getOpenFileName(0, "Select Sample", ".", "Sample Files (*.mha *.mhd)");
	      path = sampleFile.section('/',0,-2);
	      proxy = pqLoadDataReaction::loadData(QStringList(sampleFile));
	    }
	  }
	}
        if( proxy )
        {
          vtkFilter* sampleReader = CachedObjectBuilder::instance()->registerProductCreator(sampleId, proxy);
	  newSample = EspINAFactory::instance()->CreateSample(sampleReader, 0,path);
          EspINA::instance()->addSample(newSample);
          // TODO same code like cachedObjectBuilder::createSMFilter() - DOUBLEVECT
          QStringList values = args["Spacing"].split(",");
          if(values.size() == 3)
          {
            newSample->setSpacing(values[0].toDouble(), values[1].toDouble(), values[2].toDouble());
          }
          foreach(QString argName, args.keys())
	  {
	    if (argName != "Id" && argName != "Taxonomy" && argName != "Spacing")
	    {
	      if (!newSample->extension(argName))
	      {
		std::cout << "ERROR: Extension " << argName.toStdString() << " doesn't exist" << std::endl;
		assert(false);
	      }
	      newSample->extension(argName)->setArguments(args[argName]);
	    }
	  }
	  //ALERT: newSample is not initialize until added to espina model
	  assert(newSample->representation(LabelMapExtension::SampleRepresentation::ID));
	  dynamic_cast<LabelMapExtension::SampleRepresentation *>(newSample->representation(LabelMapExtension::SampleRepresentation::ID))->setEnable(false);
        }
        else
        {
          qWarning() << __FILE__ << ":" << __LINE__ << "File" << label << "not found.";
          return;
        }
      } // A filter
      else if( vShape[vertexId].compare("box") == 0 )
      { // A filter that can be processed
        qDebug() << "ProcessingTrace: Creating the filter " << label;
        //QStringList filterInfo = QString(vLabel[vertexId].c_str()).split("::");
        //assert(filterInfo.size() == 2);
	// if( filterInfo.at(1) == "SeedGrowSegmentationFilter")
	
	EspinaFilter *filter = EspinaPluginManager::instance()->createFilter(label,args);
	if (!filter)
	  qDebug() << "ProcessingTrace: the filter is not registered";
	  
//         IFilterFactory* factory = m_availablePlugins.value(label, NULL);
//         if( factory )
//           factory->createFilter(label, args);
//         else
//           qDebug() << "ProcessingTrace: the filter is not registered";

      } // A segmentation
      else {
        qDebug() << "ProcessingTrace: segmentation " << args["Id"] << args["Taxonomy"];
        EspINA* espina = EspINA::instance();
	Segmentation *seg = espina->segmentation(args["Id"]);
	assert(seg);
	int segId = label.section(' ',-1).toInt();
	lastId = std::max(lastId, segId);
	espina->changeId(seg, segId);
        espina->changeTaxonomy(seg, args["Taxonomy"]);
        
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
      verticesToProcess.swap(0, verticesToProcess.size()-1);
    }
  }
  
  EspINA::instance()->setLastUsedId(lastId);
  
  if (newSample)
    dynamic_cast<LabelMapExtension::SampleRepresentation *>(newSample->representation(LabelMapExtension::SampleRepresentation::ID))->setEnable(true);
}

// //-----------------------------------------------------------------------------
// void ProcessingTrace::registerPlugin(QString key, IFilterFactory* factory)
// {
//   assert( m_availablePlugins.contains(key) == false );
//   m_availablePlugins.insert(key, factory);
// }
// 
// 
// //-----------------------------------------------------------------------------
// IFilterFactory * ProcessingTrace::getRegistredPlugin(QString& key)
// {
//   assert( m_availablePlugins.contains(key));
//   return m_availablePlugins[key];
// }


//-----------------------------------------------------------------------------
void ProcessingTrace::print( std::ostream& out, ProcessingTrace::printFormat format)
{
  this->readNodes();
  if( format == graphviz )
  {
    boost::dynamic_properties dp;

    dp.property("node_id", boost::get(boost::vertex_index, m_trace));
    dp.property("label", boost::get(&VertexProperties::labelName, m_trace));
    dp.property("shape", boost::get(&VertexProperties::shape, m_trace));
    dp.property("args", boost::get(&VertexProperties::args, m_trace));
    dp.property("label", boost::get(boost::edge_name, m_trace));

    boost::write_graphviz_dp( out, m_trace, dp);
  }
  else if( format == debug)
  {
    QStringList qstrl;
//     NodeParamList args;
//     // Retrieve vertex porperties
//     boost::property_map<Graph, std::string VertexProperty::*>::type vLabel
//       = boost::get(&VertexProperty::labelName, m_trace);
//     boost::property_map<Graph, std::string VertexProperty::*>::type vArgs
//       = boost::get(&VertexProperty::args, m_trace);
//     boost::property_map<Graph, std::string VertexProperty::*>::type vShape
//       = boost::get(&VertexProperty::shape, m_trace);
//     // Iter upon vertices
//     boost::graph_traits<Graph>::vertex_iterator vi, vi_end;
//     for( boost::tie(vi, vi_end) = boost::vertices(m_trace); vi != vi_end; vi++)
//     {
//       out << *vi << " - " << vLabel[*vi] << std::endl;
//       // If it is a filter, it must parse the args
//       if( vShape[*vi].compare("box") == 0 )
//       {
//         qstrl = QString(vLabel[*vi].c_str()).split("::");
//         assert(qstrl.size() == 2);
//         QString rawArgs( vArgs[*vi].c_str() );
//         args = parseArgs( rawArgs );
//         foreach( NodeParam param, args)
//         {
//           out << "\t" << param.first.toStdString()
//               << " = " << param.second.toStdString();
//         }
//       } // A stack or a product
//       else if( vShape[*vi].compare("ellipse") == 0 )
//       {
//         if( vLabel[*vi] != "Product" )
//           out << "\tPosible stack!";
//       }
//     }
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


