/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  Jorge PeÃ±a Pastor <email>

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


#ifndef PENCILSELECTOR_H
#define PENCILSELECTOR_H

#include <common/tools/IPicker.h>
#include <QColor>


class BrushSelector
: public IPicker
{
  Q_OBJECT
public:
  enum State {CREATING, DRAWING, ERASING};
public:
  explicit BrushSelector();

  virtual QCursor cursor();
  virtual bool filterEvent(QEvent* e, EspinaRenderView *view = 0);

  void setColor(QColor color)
  {
    m_color = color;
    m_color.setAlphaF(0.8);
  }
  void setRadius(int radius);
  int radius() const {return m_radius;}

  State state() const {return m_state;}
  void changeState(State state)
  {
    if (m_state != state)
    {
      m_state = state;
      setRadius(m_radius);
      emit stateChanged(state);
    }
  }

private:
  void startSelection(int x, int y, EspinaRenderView *view);

signals:
  void stateChanged(BrushSelector::State);

private:
  int     m_radius;
  QCursor m_cursor;
  State   m_state;
  int     m_xRef, m_yRef;
  QColor  m_color;
};

#endif // PENCILSELECTOR_H