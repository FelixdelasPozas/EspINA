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

#include <string>

#include <vector>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/subgraph.hpp>

typedef std::pair<std::string, std::string> Param;
typedef std::vector<Param> ParamList;

//! Interface to trace's nodes
class ITraceNode
{
public:
  virtual void print(int indent = 0) = 0;
  virtual ParamList getArguments() = 0;
  std::string name;
  int type;// 0: Product 1: Filter
};

//! A class to represent the working trace
class ProcessingTrace
{
  typedef unsigned int NodeId;
  //! SubGraphs REQUIRE vertex and edge properties
  typedef boost::adjacency_list<
  boost::vecS
  ,boost::vecS,
  boost::directedS,
  boost::property<boost::vertex_index_t, NodeId, ITraceNode *>,
  boost::property<boost::edge_index_t, NodeId>
  > Graph;
  typedef boost::subgraph<Graph > SubGraph;
  
public:
  ProcessingTrace();
  ~ProcessingTrace(){}
  
  void addNode(const ITraceNode *node);
  void connect(
    const ITraceNode &origin
  , const ITraceNode &destination
  , const std::string &description
  );
  
  void addSubtrace(const ProcessingTrace *subTrace);
private:
  SubGraph m_trace;
  NodeId m_nodeId;
};

#endif // TRACING_H
