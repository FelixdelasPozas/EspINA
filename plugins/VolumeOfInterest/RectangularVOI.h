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

#include "filter.h" // To use TranslatorTable

class pq3DWidget;

class RectangularVOI 
: public QObject, public IVOI
{
  Q_OBJECT;
public:
  RectangularVOI();
  
  virtual Product* applyVOI(Product* product);
  virtual Product* restoreVOITransormation(Product* product);

  virtual vtkSMProxy *getProxy();
  virtual pq3DWidget *widget();
  virtual pq3DWidget *widget(int plane);
  
public slots:
  virtual void endInteraction();
  
  virtual void cancelVOI();
  
private:
  void buildRVOITable();
  Filter *buildRectangularVOIFilter(Product *input, EspinaParamList args);
  
private:
  vtkSMProxy *m_box;
  pq3DWidget *m_widget[4];
  
  TranslatorTable m_tableRVOI;
  double m_rvoi[6];
};

#endif // RECTANGULARVOI_H
