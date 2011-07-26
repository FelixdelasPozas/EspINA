/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2011  Jorge Peña <jorge.pena.pastor@gmail.com>

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

//TODO: Show bounding regions in volume view
//TODO: Show bounding regions in slice view

#ifndef COUNTINGREGION_H
#define COUNTINGREGION_H

#include "ui_CountingRegionPanel.h"

#include <QDockWidget>
#include <QMap>
#include <QList>
#include <QStringListModel>

// Forward declaration
class QAction;
class Sample;
class pqPipelineSource;

class CountingRegion: public QDockWidget, private Ui::CountingRegionPanel
{
  Q_OBJECT

public:
  static const QString ID;

  class SegmentationExtension;
  class BoundingRegion;
  class SampleExtension;
  
public:
  CountingRegion(QWidget* parent);
  
  void initializeExtension(SegmentationExtension *ext);
  
public slots:
  void focusSampleChanged(Sample *sample);
  
  void createAdaptativeRegion();
  void createRectangularRegion();
  //void onAction(QAction *action);
  
private:
  Sample *m_focusedSample;
  QMap<Sample *, QList<pqPipelineSource *> > m_regions;
  QStringListModel m_regionsModel;
};
  

#endif // COUNTINGREGION_H
