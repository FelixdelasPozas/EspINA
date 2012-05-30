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


#ifndef SEGMENTATIONINSPECTOR_H
#define SEGMENTATIONINSPECTOR_H

#include <QDialog>
#include "ui_SegmentationInspector.h"

class VolumeView;
class pqRenderView;
class Segmentation;

class SegmentationInspector
: public QWidget
, public Ui::SegmentationInspector
{
  Q_OBJECT
public:
  static SegmentationInspector *CreateInspector(Segmentation *seg);
  virtual ~SegmentationInspector();

public slots:
  void updateScene();

protected:
  SegmentationInspector(Segmentation *seg, QWidget* parent = 0, Qt::WindowFlags f = 0);
  virtual void closeEvent(QCloseEvent *e);

private:
  Segmentation *m_seg;
  VolumeView   *m_view;

  static QMap<Segmentation *, SegmentationInspector *> m_inspectors;
};

#endif // SEGMENTATIONINSPECTOR_H
