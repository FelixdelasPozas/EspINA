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

#ifndef FILTER_H
#define FILTER_H

#include "tracing.h"
#include "interfaces.h"

#include <map>
#include <vector>

// Forward declarations
class pqPipelineSource;
typedef pqPipelineSource EspinaProxy;

class ISingleton
{
public:
  virtual std::string id() = 0;
};

class Product 
: public ISelectableObject
, public ITraceNode
, public ISingleton
, public IRenderable
{
public:
  Product(){}
  virtual ~Product(){}

  //! Implements ITraceNode interface
  virtual std::vector<ITraceNode *> inputs();
  virtual std::vector<ITraceNode *> outputs();
  virtual void print(int indent = 0) const;
  virtual ParamList getArguments();
  
  //! Implements ISingleton
  virtual std::string id();
  
  //! Implements IRenderable
  virtual pqOutputPort* outPut();
  
  // Friend of Filter?
  virtual void setOutputPort(pqOutputPort *port);

private:
  pqOutputPort *m_outputPort;
};


class Filter : public ITraceNode, public ISingleton
{
public:
  typedef int vtkArg;
  typedef std::string espinaArg;
  
  class TranslatorTable{
  public: 
    ParamList translate(ParamList args) const {return args;}
  private:
    std::map<espinaArg,vtkArg> m_table;
  };
  
public:
  Filter(
    const std::string &group
  , const std::string &name
  , const ParamList &args
  , const TranslatorTable &table  
  );
  
  //! Implements ITraceNode interface
  virtual std::vector<ITraceNode *> inputs();
  virtual std::vector<ITraceNode *> outputs();
  virtual void print(int indent = 0) const;
  virtual ParamList getArguments();
  
  //! Implements ISingleton
  virtual std::string id();
  
  std::vector<Product *> products();
  
private:
  //void createFilter();
  
private:
  ParamList m_args;
  EspinaProxy *m_proxy;
  const TranslatorTable &m_translator;
  std::vector<Product *> m_products;
};

#endif // FILTER_H
