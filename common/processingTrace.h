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
#ifndef PROCESSINGTRACE_H
#define PROCESSINGTRACE_H

#include "espinaTypes.h"

#include <boost/graph/adjacency_list.hpp>

#include <QString>
/*
//! A Trace node argument. It only has semantic meaning
typedef std::string EspinaArg;
//! The value of a Espina argument
typedef std::string ParamValue;
typedef std::pair<EspinaArg, ParamValue> EspinaParam;
typedef std::vector<EspinaParam> EspinaParamList;
*/
#include <QMap>
#include <qtextstream.h>

typedef unsigned int IndexType;
//Forward declarations
class ProcessingTrace;
class IFilterFactory;

#define ESPINA_ARG(name, value) QString("%1=%2;").arg(name).arg(value)

//! Interface to trace's nodes
class ITraceNode
{
public:
  typedef QMap<QString, QString> Arguments;
  enum Shape
  { PRODUCT = 0
  , FILTER  = 1
  };
  
public:
  virtual ~ITraceNode(){}
  
  static ITraceNode::Arguments parseArgs(QString &raw);
  virtual QString getArgument(QString name) const = 0;
  virtual QString getArguments() const = 0;
  //! Descriptive name of the node
  virtual QString label() const = 0;
  
  //! Type used to enhance the output of the graph....
  Shape type;

  //! Debug function
  //virtual void print(int indent = 0) const {};
private:
  //! Node id in the graph
  //IndexType vertexId;
  friend class ProcessingTrace;
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
  boost::listS,
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
  typedef enum {graphviz, debug} printFormat;
  
  static ProcessingTrace* instance();
  ~ProcessingTrace(){}
  
  void addNode(ITraceNode* node);
  void connect(
    ITraceNode *origin
  , ITraceNode *destination
  , const std::string &description
  );
  void connect(
    QString& id,
    ITraceNode *destination,
    const std::string &description
  );
  
  //! Remove a segmentation and the filter which created it if no more
  //! segmentations exist
  void removeNode(ITraceNode* node);
    
//   void readTrace(std::istream& content);
  void readTrace(QTextStream& stream);

  void registerPlugin(QString key, IFilterFactory* factory);
  IFilterFactory* getRegistredPlugin(QString& key);
  /*
  void addSubtrace(const ProcessingTrace *subTrace);
  std::vector<ITraceNode *> inputs(const ITraceNode *node);
  std::vector<ITraceNode *> outputs(const ITraceNode *node);
  */
  void print(std::ostream& out, printFormat format = graphviz);
  
private:
  ProcessingTrace();
  ProcessingTrace(const QString &name); // TODO delte. No tiene sentido sin subgraph
  //! It retrieves the information of the ITraceNodes to store the hold trace
  void readNodes();
  //! It retrieves the current vertex index of a ITraceNode
  boost::graph_traits< Graph >::vertex_descriptor vertexIndex(ITraceNode* arg1);
  //!Convert a string int the correct format "{argument:value;}+" in a NodeParamList
  //ITraceNode::Arguments parseArgs( QString& raw );

  // attributes
  Graph m_trace;
  static ProcessingTrace* m_instnace;
  QMap<QString, IFilterFactory *> m_availablePlugins;
  
};

#endif // PROCESSINGTRACE_H
