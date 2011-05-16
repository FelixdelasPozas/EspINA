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

#include "processingTrace.h"
#include "products.h"
#include "interfaces.h"
#include "translatorTable.h"
#include "data/modelItem.h"
#include "EspinaPlugin.h"

#include <QString>
#include <data/taxonomy.h>
#include <Utilities/vxl/vcl/iso/vcl_iostream.h>

#include <QMutex>

// Forward declarations
class pqPipelineSource;
class CachedObjectBuilder;
class Sample;

class vtkProduct;

class IFilter
{
public:
  virtual ~IFilter(){}
  //! Returns the number of products created by the filter
  virtual int numProducts() = 0;
  //! Returns the i-th product created by the filter
  virtual vtkProduct product(int i) = 0;
  //! Returns all products created by the filter
  virtual QList<vtkProduct *> products() = 0;
  
  virtual QString getFilterArguments() const = 0;
};


//! Represents a unique vtk filter
class vtkFilter
: public IFilter
{
public:
  enum VtkPropType
  { UNKOWN     = -1
  , INPUT      = 0    
  , INTVECT    = 1
  , DOUBLEVECT = 2
  };
  
  struct Argument
  {
    Argument(QString newName, VtkPropType newType, QString newValue)
    : name(newName)
    , type(newType)
    , value(newValue){}
    
    QString name;
    VtkPropType type;
    QString value;
  };
  
  typedef QList<Argument> Arguments;

public:
  virtual ~vtkFilter();
  //! Implements IFilter Interface
  virtual int numProducts();
  virtual vtkProduct product(int i);
  virtual QList<vtkProduct *> products();
  virtual QString getFilterArguments() const {return "";}
  
  QString id(){return m_id;}
  pqPipelineSource *pipelineSource(){return m_pipelineSource;}

private:
  vtkFilter(pqPipelineSource *source, QString &cacheId);
  
protected:  
  pqPipelineSource *m_pipelineSource;
  QString m_id; //! Cache id
  friend class CachedObjectBuilder;
};


//! Represents a filter that can be traced
class EspinaFilter 
: public IFilter
//, public ITraceNode
{
public:
  virtual ~EspinaFilter(){}
  //virtual int numProducts() = 0;
  //virtual vtkProduct* product(int i) = 0;
  //virtual QList< vtkProduct* > products() = 0;
  virtual QString getFilterArguments() const {return m_args;}
  virtual void removeProduct(vtkProduct *product) = 0;
  
protected:
  QString m_args;
};


/*
/// DEPRECATED: Old method
class Filter : public ITraceNode, public ISingleton
{
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
  virtual void print(int indent = 0) const;
  virtual EspinaParamList getArguments();
  
  //! Implements ISingleton
  virtual EspinaId id();
  
  QString group(){return m_group;}
  VtkParamList vtkArgs(){return m_vtkArgs;}
  
  std::vector<Product *> products();
  //ProcessingTrace *trace();
  
private:
  //void createFilter();
  QString m_group;
  
  EspinaParamList m_args;
  VtkParamList m_vtkArgs;
  pqPipelineSource *m_proxy;
  const TranslatorTable &m_translator;
  //ProcessingTrace m_filtertrace;
  std::vector<Product *> m_products;
};
*/



/*
//! Represents a Filter from the VTK p.o.v.
class vtkFilter : public ISingleton
{
public:
  typedef QMap<QString, QString> Arguments;
  
  Arguments args();
  
private:
  Arguments m_args;
  QString m_group;
  QString m_name;
  
  pqPipelineSource *m_proxy;
};
*/


#endif // FILTER_H
