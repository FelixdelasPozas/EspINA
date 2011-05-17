/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2011  Jorge Peña <jorge.pena.pastor@gmail.com>

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


#ifndef RECTANGULARVOI_H
#define RECTANGULARVOI_H

#include <selectionManager.h>

#include "filter.h"

class pq3DWidget;

class RectangularVOI 
: public QObject
, public IVOI
, IFilterFactory
{
  class ApplyFilter: public EspinaFilter
  {
  public:
    virtual ~ApplyFilter();
    ApplyFilter(vtkProduct *input, double *bounds);
    ApplyFilter(ITraceNode::Arguments &args);
    virtual int numProducts() {return 1;}//Asser it is true :D
    virtual vtkProduct product(int i) {return vtkProduct(m_rvoi,0);}
    virtual QList< vtkProduct* > products() {QList<vtkProduct *> p; return p;}
    virtual QString getFilterArguments() const {return m_args;}
    virtual void removeProduct(vtkProduct* product);
    
    static const QString FilterType;
  private:
    vtkFilter *m_rvoi;
  };

  Q_OBJECT;
public:
  RectangularVOI();
  
  virtual EspinaFilter *createFilter(QString filter, ITraceNode::Arguments& args);
  
  virtual EspinaFilter *applyVOI(vtkProduct* product);
  virtual EspinaFilter *restoreVOITransormation(vtkProduct* product);

  virtual vtkSMProxy *getProxy();
  virtual pq3DWidget *widget();
  virtual pq3DWidget *widget(int plane);
  virtual pq3DWidget* newWidget();
  virtual void deleteWidget(pq3DWidget* &widget);


  
public slots:
  virtual void endInteraction();
  
  virtual void cancelVOI();
  
private:
  vtkSMProxy *m_box;
  QList<pq3DWidget *> m_widgets;
  
  double m_rvoi[6];
};

#endif // RECTANGULARVOI_H
