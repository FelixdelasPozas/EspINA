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


#ifndef SEGMENTATIONEXPLORER_H
#define SEGMENTATIONEXPLORER_H

#include <QWidget>
#include "ui_segmentationExplorer.h"

class Segmentation;
class pqRenderView;

class SegmentationExplorer : public QWidget, public Ui::SegmentationExplorer
{
  Q_OBJECT
public:
  SegmentationExplorer(Segmentation *seg, QWidget* parent = 0, Qt::WindowFlags f = 0);
  virtual ~SegmentationExplorer();
  
public slots:
  void takeSnapshot();
  void exportScene();
 
signals:
  void segmentationInformationHiden(Segmentation *);
  
private:
  pqRenderView *view;
  Segmentation *m_seg;
};

#endif // SEGMENTATIONEXPLORER_H
