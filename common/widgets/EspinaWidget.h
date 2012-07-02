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

#include <common/views/vtkSliceView.h>

class vtkAbstractWidget;

class SliceWidget
{
public:
  explicit SliceWidget(vtkAbstractWidget *widget);

  virtual void setSlice(double pos, vtkSliceView::VIEW_PLANE plane);

  //operator pq3DWidget *(){return m_widget;}
  //operator const pq3DWidget * const() const {return m_widget;}
  //   pq3DWidget *operator->() {return m_widget;}

protected:
  vtkAbstractWidget *m_widget;
};

class EspinaWidget
{
public:
  virtual ~EspinaWidget(){}

//   virtual pq3DWidget*  createWidget() = 0;
//   virtual SliceWidget *createSliceWidget(vtkSliceView::VIEW_PLANE plane) = 0;

  virtual void setEnabled(bool enable) = 0;
  /// Expand the widget to fit @bounds
  virtual void setBounds(double bounds[6]) = 0;
  virtual void bounds(double bounds[6]) = 0;
};

#endif // ESPINAWIDGET_H
