/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2011  Jorge Peña Pastor<jpena@cesvima.upm.es>

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


#include <QList>
#include <QString>

class pqData;
class pqPipelineSource;

class IFilter
{
public:
  IFilter(){};
  virtual ~IFilter(){}

  /// Returns the number of products created by the filter
  virtual int getNumberOfData() = 0;
  /// Returns the i-th data created by the filter
  virtual pqData data(int i) = 0;

  virtual QString getFilterArguments() const = 0;

private:
  IFilter(const IFilter &);        // Disable copy constructor
  void operator=(const IFilter &); // Disable copy operator
};


struct Argument
{
  enum VtkPropType
  { UNKOWN     = -1
  , INPUT      = 0
  , INTVECT    = 1
  , DOUBLEVECT = 2
  };

  Argument(QString newName, VtkPropType newType, QString newValue)
  : name(newName)
  , type(newType)
  , value(newValue){}

  QString name;
  VtkPropType type;
  QString value;
};


/// Represents a filter in Paraview's pipeline
class pqFilter
// : public IFilter
{
  friend class Cache;
  friend class CachedObjectBuilder;
public:
  typedef QList<Argument> Arguments;

  virtual ~pqFilter();
  //! Implements IFilter Interface
  virtual int getNumberOfData();
  virtual pqData data(int i);
//   virtual QList<pqData *> data();
//   virtual QString getFilterArguments() const {return "";}

  QString id() const {return m_id;}
  pqPipelineSource *pipelineSource(){return m_source;}
  void clearPipeline();

private:
  pqFilter(pqPipelineSource* source, const QString& cacheId);

protected:
  pqPipelineSource *m_source;
  QString           m_id; /// Cache id
};

/*
//! Represents a filter that can be traced
class EspinaFilter 
: public IFilter
{
public:
  virtual ~EspinaFilter(){}
  //virtual int numProducts() = 0;
  //virtual vtkProduct* product(int i) = 0;
  //virtual QList< vtkProduct* > products() = 0;
  virtual QString getFilterArguments() const = 0;
  virtual void removeProduct(vtkProduct *product) = 0;
  virtual QWidget *createWidget() = 0;

protected:
  QString m_args;
};*/


#endif // FILTER_H
