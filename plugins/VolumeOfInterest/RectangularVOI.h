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
{
  class ApplyFilter: public EspinaFilter
  {
  public:
    ApplyFilter(vtkProduct *input, double *bounds, );
    virtual int numProducts() {return 1;}//Asser it is true :D
    virtual vtkProduct product(int i) {return vtkProduct(m_rvoi,0);}
    virtual QList< vtkProduct* > products() {QList<vtkProduct *> p; return p;}
    virtual QString getFilterArguments() const {return m_args;}
  private:
    vtkFilter *m_rvoi;
  };

  Q_OBJECT;
public:
  RectangularVOI(EspinaPlugin* parent
);
  
  virtual IFilter *createApplyFilter(){}
  virtual IFilter *createRestoreFilter(){}
  
  virtual IFilter *applyVOI(vtkProduct* product);
  virtual IFilter *restoreVOITransormation(vtkProduct* product);

  virtual vtkSMProxy *getProxy();
  virtual pq3DWidget *widget();
  virtual pq3DWidget *widget(int plane);
  
public slots:
  virtual void endInteraction();
  
  virtual void cancelVOI();
  
private:
  vtkSMProxy *m_box;
  pq3DWidget *m_widget[4];
  
  double m_rvoi[6];
};

#endif // RECTANGULARVOI_H
