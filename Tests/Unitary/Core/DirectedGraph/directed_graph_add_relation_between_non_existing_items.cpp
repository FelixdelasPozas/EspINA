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

#include <Core/Analysis/AnalysisItem.h>
#include "DummyItem.h"

using namespace EspINA;
using namespace UnitTesting;
using namespace std;

int directed_graph_add_relation_between_non_existing_items( int argc, char** argv )
{
  bool error1 = true;
  bool error2 = true;

  DirectedGraph graph;
  
  DummyItemSPtr item1{new DummyItem()};
  DummyItemSPtr item2{new DummyItem()};
  DummyItemSPtr unexistingItem{new DummyItem()};
  QString       relation{"link"};
  
  graph.addItem(item1);
  
  graph.addItem(item2);
  
  try {
    graph.addRelation(item1, unexistingItem, relation);
    cerr << "Added relation to unexisting item" << endl;
  } catch (DirectedGraph::Item_Not_Found_Exception e) {
    error1 = false;
  }
  
  try {
    graph.addRelation(unexistingItem, item2, relation);
    cerr << "Added relation to unexisting item" << endl;
  } catch (DirectedGraph::Item_Not_Found_Exception e) {
    error2 = false;
  }
  
  if (!graph.edges().isEmpty()) {
    cerr << "Unexpected number of edges" << endl;
    error1 = true;    
  }

  return error1 || error2;
}