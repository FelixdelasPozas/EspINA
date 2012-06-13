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

#include <common/selection/SelectionHandler.h>


class PencilSelector
: public SelectionHandler
{
  Q_OBJECT
public:
  enum State {DRAWING, ERASING};
public:
  explicit PencilSelector(SelectionHandler* succesor = NULL);

  virtual bool filterEvent(QEvent* e, SelectableView* view = 0);
  virtual QCursor cursor();

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

signals:
  void stateChanged(PencilSelector::State);


private:
  int     m_radius;
  QCursor m_cursor;
  bool    m_tracking;
  State   m_state;
  int     m_xRef, m_yRef;
};

#endif // PENCILSELECTOR_H
