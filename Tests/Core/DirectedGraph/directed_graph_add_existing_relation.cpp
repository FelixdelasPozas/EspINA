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

#include <Core/Analysis/Persistent.h>
#include "DummyItem.h"

using namespace ESPINA;
using namespace UnitTesting;
using namespace std;

int directed_graph_add_existing_relation(int argc, char** argv)
{
  bool error = false;

  DirectedGraph graph;

  auto item1 = std::make_shared<DummyItem>();
  auto item2 = std::make_shared<DummyItem>();
  QString relation
  { "link" };

  graph.add(item1);
  graph.add(item2);
  graph.addRelation(item1, item2, relation);

  try
  {
    graph.addRelation(item1, item2, relation);
    cerr << "Expected existing relation exception" << endl;
    error = true;
  }
  catch (...)
  {
  }

  auto ref1 = item1;
  auto ref2 = item2;

  try
  {
    graph.addRelation(ref1, ref2, relation);
    cerr << "Expected existing relation exception" << endl;
    error = true;
  }
  catch (...)
  {
  }

  return error;
}
