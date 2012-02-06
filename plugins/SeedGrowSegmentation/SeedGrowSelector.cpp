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


#include "SeedGrowSelector.h"

#include "gui/ThresholdAction.h"

#include <QWheelEvent>
#include <selection/SelectableView.h>
#include <pqRenderViewBase.h>
#include "SeedGrowSegmentationFilter.h"

SeedGrowSelector::SeedGrowSelector(ThresholdAction* th, SelectionHandler* succesor)
: SelectionHandler(succesor)
, m_threshold(th)
, filter(NULL)
{
  Q_ASSERT(m_threshold);
}

bool SeedGrowSelector::filterEvent(QEvent* e, SelectableView* view)
{
  if (e->type() == QEvent::Wheel)
  {
    QWheelEvent* we = dynamic_cast<QWheelEvent*>(e);
    if (we->modifiers() == Qt::CTRL)
    {
      int numSteps = we->delta()/8/15;//Refer to QWheelEvent doc.
      m_threshold->setThreshold(m_threshold->threshold() + numSteps);//Using stepBy highlight the input text
      view->view()->forceRender();

      return true;
    }
  }else if(e->type() == QEvent::MouseMove)
  {
    QMouseEvent *me = dynamic_cast<QMouseEvent*>(e);
    if (me->modifiers() == Qt::CTRL)
    {
      int x, y;
      view->eventPosition(x,y);
      double pick[3];
      view->pickChannel(x,y,pick);
      int seed[3] = {pick[0], pick[1], pick[2]}; 
      if (filter)
      {
	filter->setSeed(seed);
	view->view()->forceRender();
      }
      
    }
  }else if(e->type() == QEvent::MouseButtonPress)
  {
    QMouseEvent *me = dynamic_cast<QMouseEvent*>(e);
    if (me->modifiers() != Qt::CTRL && m_succesor)
    {
      return m_succesor->filterEvent(e,view);
    }
  }
  return false;
}

QCursor SeedGrowSelector::cursor()
{
  if (m_succesor)
    return m_succesor->cursor();
  else
    return m_cursor;
}
