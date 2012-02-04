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

#ifndef PQFILTER_H
#define PQFILTER_H

#include <QList>
#include <QString>
#include <QDebug>

class pqData;
class pqPipelineSource;

/// Represents a filter in Paraview's pipeline
class pqFilter
// : public IFilter
{
public:
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

  friend class Cache;
  friend class CachedObjectBuilder;
};


QDebug operator<<(QDebug &out, const pqFilter::Argument &arg);



#endif // PQFILTER_H
