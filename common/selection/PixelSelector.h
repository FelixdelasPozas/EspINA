/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2011  Jorge Peña Pastor<jpena@cesvima.upm.es>

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

#ifndef PIXELSELECTOR_H_
#define PIXELSELECTOR_H_

#include "SelectionManager.h"

class PixelSelector 
: public SelectionHandler
{
public:
  PixelSelector(SelectionHandler *succesor=NULL)
  : SelectionHandler(succesor)
  , m_handled(true) {}
  virtual ~PixelSelector(){}

  virtual void onMouseDown(const QPoint &pos, SelectableView* view);
  virtual void onMouseMove(const QPoint &pos, SelectableView* view);
  virtual void onMouseUp  (const QPoint &pos, SelectableView* view);
  virtual bool filterEvent(QEvent* e, SelectableView* view = 0);

  void setHandleEvent(bool handled) {m_handled = handled;}

private:
  bool m_handled;
};

class QSize;

class BestPixelSelector
: public PixelSelector
{
public:
  BestPixelSelector(SelectionHandler *succesor=NULL);
  virtual ~BestPixelSelector(){}

  void setBestPixelValue(int value) {m_bestPixel = value;}

  virtual void onMouseDown(const QPoint& pos, SelectableView* view);

private:
  QSize *m_window;
  int    m_bestPixel;
};

#endif //PIXELSELECTOR_H_
