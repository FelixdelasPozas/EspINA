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

class Channel;
class ViewManager;
class BrushSelector;

class Brush
: public ITool
{
  Q_OBJECT
public:
  enum Mode {CREATE, MODIFY};

public:
  explicit Brush(ViewManager *vm);
  virtual ~Brush();

  virtual QCursor cursor() const;
  virtual bool filterEvent(QEvent* e, EspinaRenderView* view = 0);
  virtual void setEnabled(bool enable);
  virtual void setInteraction(bool enable);
  virtual bool interactive() const;

private:
  void buildCursor();
  void processTrack(QList<QPoint> dots, EspinaRenderView *view);

signals:
  void brushCenters(Channel *channel, IPicker::WorldRegion centers, Nm radius, PlaneType plane);
  void eraserCenters(Channel *channel, IPicker::WorldRegion centers, Nm radius, PlaneType plane);
  void stopDrawing();

private:
  ViewManager *m_viewManager;

  Mode    m_mode;
  bool    m_erasing;
  bool    m_tracking;
  Nm      m_radius;
  QCursor m_cursor;
  QList<QPoint> m_dots;

  static const int MAX_RADIUS = 32;
};

#endif // BRUSH_H
