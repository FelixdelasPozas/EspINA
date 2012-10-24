/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  <copyright holder> <email>

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

#include "common/tools/IVOI.h"

#include "common/gui/ViewManager.h"
#include "common/tools/PixelSelector.h"
#include "common/widgets/RectangularRegion.h"

class RectangularVOI
: public IVOI
{
  Q_OBJECT
public:
  explicit RectangularVOI(ViewManager *vm);
  virtual ~RectangularVOI();

  virtual QCursor cursor() const;
  virtual bool filterEvent(QEvent* e, EspinaRenderView* view = 0);
  virtual void setInUse(bool enable);
  virtual void setEnabled(bool enable);
  virtual bool enabled() const {return m_interactive;}

  virtual IVOI::Region region();

private slots:
  void defineVOI(IPicker::PickList channels);
  void setBorder(Nm pos, PlaneType plane, ViewManager::SliceSelectors flags);

private:
  ViewManager *m_viewManager;

  bool m_enabled;
  bool m_interactive;

  PixelSelector      m_picker;
  RectangularRegion *m_widget;
  double             m_bounds[6];
};

#endif // RECTANGULARVOI_H
