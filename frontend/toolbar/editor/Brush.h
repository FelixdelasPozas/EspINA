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


#ifndef BRUSH_H
#define BRUSH_H

#include "common/tools/ITool.h"
#include <tools/IPicker.h>
#include <EspinaTypes.h>
#include "common/model/Segmentation.h"

class EspinaModel;
class QUndoStack;
class ViewManager;
class BrushPicker;
class Channel;
class Filter;
class Segmentation;

class Brush
: public ITool
{
  Q_OBJECT
public:
  enum Mode {CREATE, MODIFY};
  class DrawCommand;

public:
  explicit Brush(EspinaModel *model,
                 QUndoStack  *undoStack,
                 ViewManager *viewManager);
  virtual ~Brush();

  virtual QCursor cursor() const;
  virtual bool filterEvent(QEvent* e, EspinaRenderView* view = 0);
  virtual bool enabled() const;
  virtual void setEnabled(bool enable);
  virtual void setInUse(bool enable);

protected:
  virtual SegmentationList selectedSegmentations() const;

protected slots:
  virtual void drawStroke(PickableItem *item,
                          IPicker::WorldRegion centers,
                          Nm radius,
                          PlaneType plane) = 0;
virtual void drawStrokeStep(PickableItem *item,
                            double x, double y, double z,
                            Nm radius,
                            PlaneType plane) = 0;
signals:
  void stopDrawing();

protected:
  EspinaModel *m_model;
  QUndoStack  *m_undoStack;
  ViewManager *m_viewManager;

  Mode         m_mode;
  bool         m_erasing;
  BrushPicker *m_brush;

  Filter       *m_currentSource;
  Segmentation *m_currentSeg;
};

#endif // BRUSH_H
