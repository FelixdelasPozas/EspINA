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
#include <string.h>
#include <boost/graph/adjacency_list.hpp>
#include <iostream>

using namespace boost;


void TraceNode::printSelf()
{
  std::cout << name << "\n";
}

void Filter::printSelf()
{
    TraceNode::printSelf();
    std::cout << "\tNum Args: " << numParam << "\n";
}


typedef adjacency_list<vecS,vecS,directedS,TraceNode> Graph;

class ProcessingTrace::Trace : public Graph
{
};

ProcessingTrace::ProcessingTrace()
{
  m_trace = new Trace();
}

ProcessingTrace::ProcessingTrace(const char *filename)
{
  strcpy(m_filename,filename);
  std::cout << m_filename << "\n";
}

ProcessingTrace::~ProcessingTrace()
{
}




