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
#ifndef TRACING_H
#define TRACING_H

#include "data/cajalTypes.h"

#include <boost/graph/adjacency_list.hpp>
//#include <boost/graph/subgraph.hpp>

#include <QString>
/*
//! A Trace node argument. It only has semantic meaning
typedef std::string EspinaArg;
//! The value of a Espina argument
typedef std::string ParamValue;
typedef std::pair<EspinaArg, ParamValue> EspinaParam;
typedef std::vector<EspinaParam> EspinaParamList;
*/
typedef NodeParamList EspinaParamList;
typedef unsigned int IndexType;
//Forward declarations
class ProcessingTrace;

//! Interface to trace's nodes
class ITraceNode
{
public:
  /*
  virtual std::vector<ITraceNode *> inputs() = 0;
  virtual std::vector<ITraceNode *> outputs() = 0;
  */
  virtual void print(int indent = 0) const = 0;
  virtual EspinaParamList getArguments() = 0;
  
  //! Descriptive name of the node
  QString name;
  //! Node id in the graph
  IndexType vertexId;
  //! Type used to enhance the output of the graph....
  int type;// 0: Product 1: Filter
};


//! A class to represent the working trace
class ProcessingTrace
{
  //typedef unsigned int IndexType;
  
  //! A set of properties for graph vertex
  struct VertexProperty
  {
    //! A pointer to a class representing
    //! a node element
    ITraceNode *node;//TODO: Use smart pointers?
    std::string labelName;
    std::string shape;
    std::string args;
  };
  typedef boost::property<
  boost::vertex_index1_t, IndexType,
  VertexProperty
  > VertexProperties;
  
  //! A set of properties for graph edges
  struct EdgeProperty
  {
    //! Describes the relationships between
    //! two nodes connected by an edge
    std::string relationship;
  };
  typedef boost::property<
  boost::edge_name_t, std::string
  > EdgeProperties;
  
  //! A set of properties for graphs
  struct GraphPropery
  {
    //! The name of the class responsible 
    //! for the trace
    std::string owner;
  };
  typedef GraphPropery GraphProperties;
  
  //! SubGraphs REQUIRE vertex and edge properties
  typedef boost::adjacency_list<
  boost::vecS,
  boost::vecS,
  boost::directedS,
  VertexProperties,
  EdgeProperties,
  GraphProperties
  > Graph;
  
  //typedef boost::subgraph<SubGraph> Graph;
  
  typedef boost::graph_traits<Graph>::out_edge_iterator OutEdgeIter;
  typedef std::pair<OutEdgeIter, OutEdgeIter> EdgeIteratorRange;
  
  typedef Graph::vertex_descriptor VertexId;
  typedef Graph::edge_descriptor EdgeId;
  
public:
  static ProcessingTrace* instance();
  ~ProcessingTrace(){}
  
  void addNode(ITraceNode* node);
  void connect(
    ITraceNode *origin
  , ITraceNode *destination
  , const std::string &description
  );
  
  void readTrace(std::istream& fileName);
  
  /*
  void addSubtrace(const ProcessingTrace *subTrace);
  std::vector<ITraceNode *> inputs(const ITraceNode *node);
  std::vector<ITraceNode *> outputs(const ITraceNode *node);
  */
  void print(std::ostream& out);
  
private:
  ProcessingTrace();
  ProcessingTrace(const QString &name); // TODO delte. No tiene sentido sin subgraph
  
  Graph m_trace;
  static ProcessingTrace* m_instnace;
};

#endif // TRACING_H
