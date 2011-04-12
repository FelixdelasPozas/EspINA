#ifndef GRAPHHELPER_H
#define GRAPHHELPER_H

#include <QList>
#include <boost/graph/graphviz.hpp>
#include <qdebug.h>
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


#endif // GRAPHHELPER_H