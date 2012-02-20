/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2011  Jorge Peña Pastor <jpena@cesvima.es>

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

#include <../Views/vtkPVSliceView.h>
#include <selection/SelectionHandler.h>

class pq3DWidget;
class vtkSMProxy;

class EspinaWidget
{
public:
  virtual pq3DWidget* createWidget(vtkPVSliceView::VIEW_PLANE plane) = 0;
};

class RectangularRegion
: public EspinaWidget
{
public:
  explicit RectangularRegion();

  virtual pq3DWidget* createWidget(vtkPVSliceView::VIEW_PLANE plane);

  vtkSMProxy *getProxy();
//   virtual void setDefaultBounds(double bounds[6]);
//   virtual void resizeToDefaultSize();
//   void bounds ( double bounds[6] );

//   virtual vtkSMProxy *getProxy();
//   virtual void deleteWidget(pq3DWidget* &widget);

//   virtual bool contains(SelectionHandler::VtkRegion region);
//   virtual bool intersectPlane(ViewType plane, int slice);

//   virtual void setEnabled(bool value);

// protected slots:
//   virtual void modifyVOI();

// private:
//   void rvoiExtent(double *rvoi);

private:
  vtkSMProxy *m_box;
//   QList<pq3DWidget *> m_widgets;
};

#endif // RECTANGULARVOI_H
