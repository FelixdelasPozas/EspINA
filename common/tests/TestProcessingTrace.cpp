// #include <stdio.h>
// #include <stdlib.h>
// 
// #include "processingTrace.h"
// #include <boost/graph/adjacency_list.hpp>
// #include <string.h>

#include "filter.h"

#include <iostream>

using namespace boost;
  
int main(int argc, char **argv)
{
  std::cout << "INIT" << std::endl;
  ProcessingTrace trace;
   
   ParamList args;
   Filter::TranslatorTable tb;
   Filter *blur = new Filter("filters","blur",args,tb);
   
   trace.addNode(blur);
   trace.addNode(blur);
   trace.addNode(blur);
   trace.addNode(blur);
  
//   // Load initial source
//   Source *peque = trace.createSource("peque.mha");
//   
//   // Construct plugin trace demo
//   Filter *blur = new Filter();
//   blur->name = "Blur";
//   blur->numParam = 1;
//   //blur->params.insert("sigma","6");
// 
//   trace.addNode(blur);
//   trace.connect(peque,blur,"origin");
//   
//   Source *blurredOrigin = new Source();
//   blurredOrigin->name = "Blurred (" + peque->name + ")";
//   
//   trace.addNode(blurredOrigin);
//   trace.connect(blur,blurredOrigin,"origin");
//   
// //   Filter *seedGrow = new Filter; 
// //   seedGrow->name = "SeedGrow";
// //   seedGrow->numParam = 2;
//   
//   
//   
//   
//   //TraceNode *node = &blur;
//   //node->printSelf();
//   
//   Graph g;
//   Vertex v = add_vertex(g);
//   //g[v] = &blur;
//   //std::cout << g[v]->name << std::endl;
//   //g[v]->printSelf();
//   
  
  return 0;
}
