/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  Jorge Peña Pastor <jpena@cesvima.upm.es>

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


#ifndef ESPINAWIDGET_H
#define ESPINAWIDGET_H

#include <common/EspinaTypes.h>

class vtkAbstractWidget;

class SliceWidget
{
public:
  explicit SliceWidget(vtkAbstractWidget *widget);
  virtual ~SliceWidget() {};

  virtual void setSlice(Nm pos, PlaneType plane) {};

  operator vtkAbstractWidget *(){return m_widget;}
  operator const vtkAbstractWidget *const() const {return m_widget;}
  vtkAbstractWidget *operator->() {return m_widget;}

protected:
  vtkAbstractWidget *m_widget;
};

class EspinaWidget
{
public:
  virtual ~EspinaWidget(){}

  virtual vtkAbstractWidget *createWidget() = 0;
  virtual void deleteWidget(vtkAbstractWidget *widget) = 0;
  virtual SliceWidget *createSliceWidget(PlaneType plane) = 0;

  virtual void setEnabled(bool enable) = 0;
};

#endif // ESPINAWIDGET_H
