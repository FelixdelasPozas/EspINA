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


#ifndef RECTANGULARREGIONSLICESELECTOR_H
#define RECTANGULARREGIONSLICESELECTOR_H

#include "common/gui/SliceSelectorWidget.h"

class QPushButton;
class RectangularRegion;

class RectangularRegionSliceSelector
: public SliceSelectorWidget
{
  Q_OBJECT
public:
  explicit RectangularRegionSliceSelector(RectangularRegion *region);
  virtual ~RectangularRegionSliceSelector();

  virtual void setPlane(const PlaneType plane);

  virtual QWidget *leftWidget () const;
  virtual QWidget *rightWidget() const;

  void setLeftLabel (const QString &label) {m_leftLabel  = label; update();}
  void setRightLabel(const QString &label) {m_rightLabel = label; update();}

  virtual SliceSelectorWidget *clone();

protected slots:
  void update();
  void leftWidgetClicked();
  void rightWidgetClicked();

private:
  RectangularRegion *m_region;

  QPushButton *m_leftWidget;
  QPushButton *m_rightWidget;

  QString m_leftLabel;
  QString m_rightLabel;
};

#endif // RECTANGULARREGIONSLICESELECTOR_H
