/*
 * Copyright (c) 2013, Jorge Peña Pastor <jpena@cesvima.upm.es>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *     * Neither the name of the <organization> nor the
 *     names of its contributors may be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY Jorge Peña Pastor <jpena@cesvima.upm.es> ''AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL Jorge Peña Pastor <jpena@cesvima.upm.es> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include "Core/Analysis/Graph/DirectedGraph.h"

#include "DummyItem.h"

using namespace ESPINA;
using namespace UnitTesting;
using namespace std;

using Vertex = DirectedGraph::Vertex;

int directed_graph_expected_edges( int argc, char** argv )
{
  bool error = false;

  DirectedGraph graph;

  DummyItemSPtr item1{new DummyItem()};
  DummyItemSPtr item2{new DummyItem()};
  DummyItemSPtr item3{new DummyItem()};

  QString link   {"link"   };
  QString input1 {"input1" };
  QString input2 {"input2" };
  QString output1{"output1"};
  QString output2{"output2"};

  graph.add(item1);
  graph.add(item2);
  graph.add(item3);

  graph.addRelation(item1, item2, input1);
  graph.addRelation(item1, item2, input2);

  graph.addRelation(item2, item3, output1);
  graph.addRelation(item2, item3, output2);

  graph.addRelation(item1, item2, link);
  graph.addRelation(item2, item3, link);

  if (graph.vertices().size() != 3) {
    cerr << "Unexpected number of vertices" << endl;
    error = true;
  }

  if (graph.edges().size() != 6) {
    cerr << "Unexpected number of edges" << endl;
    error = true;
  }

  if (graph.edges(item1).size() != 3) {
    cerr << "Unexpected number of edges for vertex " << endl;
    error = true;
  }

  if (graph.edges(item2).size() != 6) {
    cerr << "Unexpected number of edges for vertex " << endl;
    error = true;
  }

  if (graph.edges(item3).size() != 3) {
    cerr << "Unexpected number of edges for vertex " << endl;
    error = true;
  }

  return error;
}