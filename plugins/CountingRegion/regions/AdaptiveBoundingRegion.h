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


#ifndef ADAPTIVEBOUNDINGREGION_H
#define ADAPTIVEBOUNDINGREGION_H

#include "regions/BoundingRegion.h"

class Channel;
class AdaptiveBoundingRegion
: public BoundingRegion
{
  Q_OBJECT
public:
  explicit AdaptiveBoundingRegion(CountingRegionSampleExtension *sampleExt,
				  Channel *channel,
				  double inclusion[3],
				  double exclusion[3]);
  virtual ~AdaptiveBoundingRegion();

  // Implements QStandardItem interface
  virtual QVariant data(int role = Qt::UserRole + 1) const;

  // Implements EspinaWidget itnerface
  virtual pq3DWidget  *createWidget();
  virtual SliceWidget *createSliceWidget(vtkPVSliceView::VIEW_PLANE plane);
  virtual void setBounds(double bounds[6]);
  virtual void bounds(double bounds[6]);
  virtual void setEnabled(bool enable);

protected slots:
  void resetWidgets();

private:
  pq3DWidget *createWidget(QString name);

private:
  QList<pq3DWidget *> m_widgets;
};

#endif // ADAPTIVEBOUNDINGREGION_H
