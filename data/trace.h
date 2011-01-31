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

#include <map>
#include <string>

//! Interface to trace elements
class TraceNode
{
public:
  virtual void printSelf(int indent = 0);
  std::string name;
  
};

class TraceConnection
{
public:
  std::string description;
};


//! A class to represent a filter as part of a trace
class Filter2 : public TraceNode
{
public:
  void printSelf(int indent = 0);
  int numParam;
  std::map<std::string,std::string> params;
};

class Source : public TraceNode
{
};

//! A class to represent how products are related to filters 
class ProcessingTrace
{
  class Trace;
public:
  ProcessingTrace();
  ProcessingTrace(const char *filename);
  ~ProcessingTrace();
  
  void addNode(TraceNode *node){}
    Source *createSource(const char* arg1){}
    void connect(TraceNode* arg1, TraceNode *arg2, const char* arg3){}
  
  //void addTrace();

private:
  std::string m_filename;
  Trace *m_trace;
};

#endif // PROCESSINGTRACE_H
