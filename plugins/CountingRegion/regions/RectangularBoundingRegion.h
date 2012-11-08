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

class CountingRegionChannelExtension;

class RectangularBoundingRegion
: public BoundingRegion
{
public:
  static const QString ID;

  explicit RectangularBoundingRegion(CountingRegionChannelExtension *channelExt,
                                     Nm borders[6],
                                     Nm inclusion[3],
                                     Nm exclusion[3],
                                     ViewManager *vm);
  virtual ~RectangularBoundingRegion();

  // Implements QStandardItem interface
  virtual QVariant data(int role = Qt::UserRole + 1) const;
  virtual QString serialize() const;
  virtual QString regionType() const { return tr("Rectangular Region"); }

  // Implements EspinaWidget interface
  virtual vtkAbstractWidget *createWidget();
  virtual void deleteWidget(vtkAbstractWidget* widget);
  virtual SliceWidget *createSliceWidget(PlaneType plane);

  virtual bool processEvent(vtkRenderWindowInteractor* iren,
                            long unsigned int event);
  virtual void setEnabled(bool enable);

  virtual void updateBoundingRegionImplementation();

protected:
  vtkSmartPointer<vtkPolyData> createRectangularRegion(Nm left,
                                                       Nm top,
                                                       Nm upper,
                                                       Nm right,
                                                       Nm bottom,
                                                       Nm lower);

private:
  Nm m_borders[6];
};

#endif // RECTANGULARBOUNDINGREGION_H
