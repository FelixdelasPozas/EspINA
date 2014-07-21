/*
    
    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>

    This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
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


#ifndef SEEDGROWSEGMENTATIONFILTERINSPECTOR_H
#define SEEDGROWSEGMENTATIONFILTERINSPECTOR_H

#include <Core/Model/Filter.h>
#include "ui_SGSFilterInspectorWidget.h"

class QUndoStack;

namespace EspINA
{
class SeedGrowSegmentationFilter;
class ViewManager;

class SGSFilterInspector
: public Filter::FilterInspector
{
  class Widget;

public:
  explicit SGSFilterInspector(SeedGrowSegmentationFilter *filter);

  virtual QWidget* createWidget(QUndoStack* stack, ViewManager* viewManager);

private:
  SeedGrowSegmentationFilter *m_filter;
};


class RectangularRegion;
class RectangularRegionSliceSelector;

class SGSFilterInspector::Widget
: public QWidget
, Ui::SGSFilterInspectorWidget
{
  Q_OBJECT
public:
  explicit Widget(Filter *filter,
                  QUndoStack  *undoStack,
                  ViewManager *vm);
  virtual ~Widget();

  virtual bool eventFilter(QObject* sender, QEvent* e );

protected slots:
  void redefineROI(double *bounds);
  void modifyFilter();
  void updateRegionBounds();
  void modifyCloseValue(int);
  void modifyCloseCheckbox(int);
  void updateWidget();

private:
  QUndoStack  *m_undoStack;
  ViewManager *m_viewManager;

  SeedGrowSegmentationFilter *m_filter;
  RectangularRegion          *m_region;
  //RectangularRegionSliceSelector *m_sliceSelctor;

  int m_closeValue;
  Nm  m_voiBounds[6];
};

} // namespace EspINA

#endif // SEEDGROWSEGMENTATIONFILTERINSPECTOR_H
