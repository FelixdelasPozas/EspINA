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
, m_preview(NULL)
{
  Q_ASSERT(m_threshold);
  m_filters.clear();
  m_filters << SelectionHandler::EspINA_Channel;
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
      if (m_preview)
      {
	m_preview->setThreshold(m_threshold->threshold());
      }
      view->view()->forceRender();

      return true;
    }
  }else if(e->type() == QEvent::MouseMove)
  {
    QMouseEvent *me = dynamic_cast<QMouseEvent*>(e);
    if (me->modifiers() == Qt::SHIFT)
    {
      ViewRegions regions;
      QPolygon singlePixel;

      int xPos, yPos;
      view->eventPosition(xPos, yPos);
      singlePixel << QPoint(xPos,yPos);
      regions << singlePixel;

      MultiSelection sel = view->select(m_filters, regions);
      if (sel.size() == 0)
	return false;

      Q_ASSERT(sel.size() == 1);// Only one element selected
      SelectionHandler::Selelection element = sel.first();

      pqData input = element.second;

      Q_ASSERT(element.first.size() == 1); // with one pixel
      QVector3D pick = element.first.first();
      int seed[3] = {pick.x(), pick.y(), pick.z()};
      if (NULL == m_preview)
      {
// 	  const int W = 40;
// 	  int VOI[6] = {seed[0] - W, seed[0] + W,
// 	  seed[1] - W, seed[1] + W,
// 	  seed[2] - W, seed[2] + W};

        int VOI[6];
	view->previewExtent(VOI);
	m_preview = new SeedGrowSegmentationFilter(input, seed, m_threshold->threshold(), VOI);
	view->addPreview(m_preview);
      }
      else
      {
	m_preview->setInput(input);
	m_preview->setSeed(seed);
      }
      view->view()->forceRender();
    }else
    {
      if (m_preview)
      {
	view->removePreview(m_preview);
	delete m_preview;
	m_preview = NULL;
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

void SeedGrowSelector::previewOn()
{
  
}

void SeedGrowSelector::previewOff()
{

}
