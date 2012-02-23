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


#ifndef RECTANGULARBOUNDINGREGION_H
#define RECTANGULARBOUNDINGREGION_H

#include "regions/BoundingRegion.h"

class RectangularBoundingRegion
: public BoundingRegion
{
  Q_OBJECT
public:
  explicit RectangularBoundingRegion(double borders[6],
				     int left,  int top,    int upper,
				     int right, int bottom, int lower);
  virtual ~RectangularBoundingRegion();

  // Implements QStandardItem interface
  virtual QVariant data(int role = Qt::UserRole + 1) const;

  // Implements EspinaWidget interface
  virtual pq3DWidget* createWidget();
  virtual pq3DWidget* createSliceWidget(vtkPVSliceView::VIEW_PLANE plane);
  virtual void setBounds(double bounds[6]);
  virtual void setEnabled(bool enable);

protected slots:
  void resetWidgets();

private:
  QList<pq3DWidget *> m_widgets;
};

#endif // RECTANGULARBOUNDINGREGION_H
