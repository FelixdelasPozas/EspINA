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

#include "model/ModelItem.h"

#include <iostream>
#include <boost/graph/graphviz.hpp>
#include "Filter.h"

#include <QDebug>

using namespace boost;

const std::string BOX = "box";
const std::string ELLIPSE = "ellipse";
const std::string INVTRIANGLE = "invtriangle";
const std::string TRAPEZIUM = "trapezium";

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
  //node->vertexId = v;
  m_graph[v].item = item;
  item->m_vertex = &m_graph[v];
  item->m_relations = this;
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
    vertex.name = item->data(Qt::DisplayRole).toString().toStdString();
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
// 	vertex.args = filter->argments();
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
}

//-----------------------------------------------------------------------------
void RelationshipGraph::addRelation(ModelItem* ancestor,
				    ModelItem* successor,
				    const QString description)
{
  boost::add_edge(vertex(ancestor), vertex(successor), description.toStdString(), m_graph);
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
RelationshipGraph::Vertices RelationshipGraph::rootVertices()
{
  VertexId v;
  Vertices result;

  VertexIterator vi, vi_end;
  for(boost::tie(vi, vi_end) = boost::vertices(m_graph); vi != vi_end; vi++)
  {
    InEdgeIterator iei, iei_end;
    boost::tie(iei, iei_end) = boost::in_edges(*vi,m_graph);
    if (iei == iei_end)
      result << *vi;
  }

  return result;
}

//-----------------------------------------------------------------------------
RelationshipGraph::Vertices RelationshipGraph::ancestors(RelationshipGraph::VertexId v) const
{
  Vertices result;
  InEdgeIterator iei, iei_end;

  qDebug() << "Ancestors of:" << m_graph[v].name.c_str();
  for(boost::tie(iei, iei_end) = boost::in_edges(v, m_graph); iei != iei_end; iei++)
  {
    qDebug() << "\t" << source(*iei, m_graph) << m_graph[source(*iei,m_graph)].name.c_str();
    result << source(*iei, m_graph);
  }
  return result;
}

//-----------------------------------------------------------------------------
RelationshipGraph::Vertices RelationshipGraph::succesors(RelationshipGraph::VertexId v) const
{
  Vertices result;
  OutEdgeIterator oei, oei_end;

  qDebug() << "Successors of:" << m_graph[v].name.c_str();
  for(boost::tie(oei, oei_end) = boost::out_edges(v, m_graph); oei != oei_end; oei++)
  {
    qDebug() << "\t" << m_graph[target(*oei,m_graph)].name.c_str();
    result << target(*oei, m_graph);
  }
  return result;
}


//-----------------------------------------------------------------------------
bool RelationshipGraph::find(VertexProperty vp, RelationshipGraph::VertexId& foundV) const
{
  VertexIterator vi, vi_end;
  for(boost::tie(vi, vi_end) = boost::vertices(m_graph); vi != vi_end; vi++)
  {
    if (m_graph[*vi].name  == vp.name  &&
        m_graph[*vi].shape == vp.shape &&
        m_graph[*vi].args  == vp.args)
    {
      foundV = *vi;
      return true;
    }
  }
  return false;
}

//-----------------------------------------------------------------------------
QString RelationshipGraph::name(RelationshipGraph::VertexId v) const
{
  return QString(m_graph[v].name.c_str());
}

//-----------------------------------------------------------------------------
ModelItem::ItemType RelationshipGraph::type(RelationshipGraph::VertexId v) const
{
  if (m_graph[v].item)
    return m_graph[v].item->type();
  else if (m_graph[v].shape == TRAPEZIUM)
    return ModelItem::SAMPLE;
  else if (m_graph[v].shape == BOX)
    return ModelItem::CHANNEL;
  else if (m_graph[v].shape == ELLIPSE)
    return ModelItem::SEGMENTATION;
  else if (m_graph[v].shape == INVTRIANGLE)
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
//   m_graph.clear(); // Reset the old trace

  boost::dynamic_properties dp;

  dp.property("node_id", boost::get(boost::vertex_index1, m_graph));
  dp.property("label", boost::get(&VertexProperties::name, m_graph));
  dp.property("shape", boost::get(&VertexProperties::shape, m_graph));
  dp.property("args", boost::get(&VertexProperties::args, m_graph));
  dp.property("label", boost::get(boost::edge_name, m_graph));

  boost::read_graphviz(serialization.string()->toStdString(), m_graph, dp);

  // Retrieve vertex porperties
//   boost::property_map<Graph, std::string VertexProperty::*>::type vLabel
//     = boost::get(&VertexProperty::name, m_graph);
//   boost::property_map<Graph, std::string VertexProperty::*>::type vArgs
//     = boost::get(&VertexProperty::args, m_graph);
//   boost::property_map<Graph, std::string VertexProperty::*>::type vShape
//     = boost::get(&VertexProperty::shape, m_graph);

//   QMap< VertexId, QList< VertexId > > parentsMap = predecessors<Graph, VertexId>(m_graph);
//   QList<VertexId> verticesToProcess(rootVertices<Graph, VertexId>(m_graph)); // The nodes not processed
//   QList<VertexId>ser;

//   Q_ASSERT(false/*ToDo*/);
/*  
  Sample *newSample;
  
  int lastId = 0;

  while( !verticesToProcess.empty() )
  {
    VertexId vertexId = verticesToProcess.first();
    qDebug() << "RelationshipGraph: Processing the vertex with id: " << vertexId;
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
      TraceNode::Arguments args = TraceNode::parseArgs( rawArgs );
      // Is a stack //TODO be more explicit
      if( vShape[vertexId].compare("ellipse") == 0 && label.contains(".") ) //Samples contains extension's dot
      {
        qDebug() << "RelationshipGraph: Loading the Stack " << label;
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
        qDebug() << "RelationshipGraph: Creating the filter " << label;
        //QStringList filterInfo = QString(vLabel[vertexId].c_str()).split("::");
        //assert(filterInfo.size() == 2);
	// if( filterInfo.at(1) == "SeedGrowSegmentationFilter")
	
	EspinaFilter *filter = EspinaPluginManager::instance()->createFilter(label,args);
	if (!filter)
	  qDebug() << "RelationshipGraph: the filter is not registered";
	  
//         IFilterFactory* factory = m_availablePlugins.value(label, NULL);
//         if( factory )
//           factory->createFilter(label, args);
//         else
//           qDebug() << "RelationshipGraph: the filter is not registered";

      } // A segmentation
      else {
        qDebug() << "RelationshipGraph: segmentation " << args["Id"] << args["Taxonomy"];
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
    dynamic_cast<LabelMapExtension::SampleRepresentation *>(newSample->representation(LabelMapExtension::SampleRepresentation::ID))->setEnable(true);*/
}

// //-----------------------------------------------------------------------------
// void RelationshipGraph::registerPlugin(QString key, IFilterFactory* factory)
// {
//   assert( m_availablePlugins.contains(key) == false );
//   m_availablePlugins.insert(key, factory);
// }
// 
// 
// //-----------------------------------------------------------------------------
// IFilterFactory * RelationshipGraph::getRegistredPlugin(QString& key)
// {
//   assert( m_availablePlugins.contains(key));
//   return m_availablePlugins[key];
// }


// //-----------------------------------------------------------------------------
// void RelationshipGraph::print(std::ostream& out, RelationshipGraph::PrintFormat format)
// {
//   this->updateVertexInformation();
// 
//   switch (format)
//   {
//     case GRAPHVIZ:
//     {
//       boost::dynamic_properties dp;
// 
//       dp.property("node_id", boost::get(boost::vertex_index, m_graph));
//       dp.property("label", boost::get(&VertexProperties::name, m_graph));
//       dp.property("shape", boost::get(&VertexProperties::shape, m_graph));
//       dp.property("args", boost::get(&VertexProperties::args, m_graph));
//       dp.property("label", boost::get(boost::edge_name, m_graph));
// 
//       boost::write_graphviz_dp( out, m_graph, dp);
//       break;
//     }
//     default:
//       qWarning("Format Unkown");
//   };
// }
 
//-----------------------------------------------------------------------------
void RelationshipGraph::serialize(std::ostream &stream, RelationshipGraph::PrintFormat format)
{
  this->updateVertexInformation();

  switch (format)
  {
    case GRAPHVIZ:
    {
      boost::dynamic_properties dp;

      dp.property("node_id"    , boost::get(boost::vertex_index, m_graph));
      dp.property("label", boost::get(&VertexProperties::name, m_graph));
      dp.property("shape",       boost::get(&VertexProperties::shape, m_graph));
      dp.property("args",        boost::get(&VertexProperties::args, m_graph));
      dp.property("label",   boost::get(boost::edge_name, m_graph));

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


