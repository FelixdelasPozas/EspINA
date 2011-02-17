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
#include "translatorTable.h"
#include "data/modelItem.h"

#include <QString>
#include <data/taxonomy.h>

// Forward declarations
class pqPipelineSource;


typedef pqPipelineSource EspinaProxy;

class ISingleton
{
public:
  virtual QString id() = 0;
};

class Product 
: public ISelectableObject
, public ITraceNode
, public ISingleton
, public IRenderable
, public IModelItem
{
public:
  Product(){}
  Product(pqPipelineSource *source, int portNumber);
  virtual ~Product(){}

  //! Implements ITraceNode interface
  virtual std::vector<ITraceNode *> inputs();
  virtual std::vector<ITraceNode *> outputs();
  virtual void print(int indent = 0) const;
  virtual EspinaParamList getArguments();
  
  //! Implements ISingleton
  virtual QString id();
  QString m_parentHash; //TODO delete
  
  //! Implements IRenderable
  virtual pqOutputPort* outputPort();
  virtual pqPipelineSource* data();	
  virtual int portNumber();
  virtual void color(double* rgba);
  
  virtual QVariant data(int role = Qt::UserRole + 1) const;
  virtual TaxonomyNode *taxonomy() {return m_taxonomy;}
  virtual void setTaxonomy(TaxonomyNode *taxonomy){m_taxonomy = taxonomy;} 
  
private:
  static int c;
  double m_rgba[4];
  TaxonomyNode *m_taxonomy;
};



class Filter : public ITraceNode, public ISingleton
{
//public:
  
//   class TranslatorTable{
//   public: 
//     //! Different Types of Vtk Properties
//     enum VtkPropType
//     { INPUT      = 0    
//     , INTVECT    = 1
//     , DOUBLEVECT = 2
//     };
//     
//     //! Argument that can be interpreted as a vtk property
//     typedef struct
//     {
//       VtkPropType type;
//       std::string name;
//     } VtkArg;
//     
//     typedef std::pair<VtkArg,ParamValue> VtkParam;
//     typedef std::vector<VtkParam> VtkParamList;
//     
//   public:
//     VtkParamList translate(EspinaParamList args) const;
//     
//   private:
//     std::map<EspinaArg,VtkArg> m_table;
//   };
  
public:
  Filter(
    //! Paraview filter's group name
    const QString &group
    //! Paraview filter's name
    , const QString &name
    //! Espina Args list
    , const EspinaParamList &args
    //! Filter Translation Table
  , const TranslatorTable &table  
  );
  
  //! Implements ITraceNode interface
  virtual std::vector<ITraceNode *> inputs();
  virtual std::vector<ITraceNode *> outputs();
  virtual void print(int indent = 0) const;
  virtual EspinaParamList getArguments();
  
  //! Implements ISingleton
  virtual QString id();
  
  std::vector<Product *> products();
  ProcessingTrace *trace();
  
private:
  //void createFilter();
  
private:
  EspinaParamList m_args;
  EspinaProxy *m_proxy;
  const TranslatorTable &m_translator;
  ProcessingTrace m_filtertrace;
  std::vector<Product *> m_products;
};

#endif // FILTER_H
