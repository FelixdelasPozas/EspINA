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

#include <common/widgets/EspinaWidget.h>
#include <vtkCommand.h>

#include <QList>
#include <QObject>

class vtkRectangularSliceWidget;

class RectangularRegion
: public QObject
, public EspinaWidget
, public vtkCommand
{
  Q_OBJECT
public:
  explicit RectangularRegion(double bounds[6]);
  virtual ~RectangularRegion();

  virtual vtkAbstractWidget* createWidget();
  virtual void deleteWidget(vtkAbstractWidget* widget);
  virtual SliceWidget* createSliceWidget(PlaneType plane);

  virtual void setEnabled(bool enable);
  virtual void setBounds(Nm bounds[6]);
  virtual void bounds(Nm bounds[6]);

  virtual void Execute(vtkObject* caller, long unsigned int eventId, void* callData);

signals:
  void modified(double *);

private:
  double m_bounds[6];
  QList<vtkRectangularSliceWidget *> m_widgets;
};

#endif // RECTANGULARVOI_H