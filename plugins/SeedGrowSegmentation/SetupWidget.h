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


#ifndef SETUPWIDGET_H
#define SETUPWIDGET_H

#include <QWidget>

#include "SeedGrowSegmentationFilter.h"
#include "ui_SetupWidget.h"
#include <common/views/vtkPVSliceView.h>


class SeedGrowSegmentationFilter::SetupWidget
: public QWidget
, Ui::SetupWidget
{
  Q_OBJECT
public:
  explicit SetupWidget(Filter *filter);
  virtual ~SetupWidget();

  virtual bool eventFilter(QObject* sender, QEvent* e );

protected slots:
  void redefineFromVOI(double value, vtkPVSliceView::VIEW_PLANE plane);
  void redefineToVOI(double value, vtkPVSliceView::VIEW_PLANE plane);
  void modifyFilter();

private :
  SeedGrowSegmentationFilter *m_filter;
};

#endif // SETUPWIDGET_H
