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

#include "common/gui/EspinaRenderView.h"
#include "common/selection/SelectableItem.h"
#include "frontend/toolbar/seedgrow/SeedGrowSegmentationFilter.h"
#include "frontend/toolbar/seedgrow/gui/ThresholdAction.h"

#include <QWheelEvent>

//-----------------------------------------------------------------------------
SeedGrowSelector::SeedGrowSelector(ThresholdAction* th, IPicker* succesor)
: IPicker(succesor)
, m_threshold(th)
, m_preview(NULL)
{
  Q_ASSERT(m_threshold);
  m_filters.clear();
  m_filters << IPicker::CHANNEL;
}

//-----------------------------------------------------------------------------
bool SeedGrowSelector::filterEvent(QEvent* e, EspinaRenderView *view)
{
  if (e->type() == QEvent::Wheel)
  {
    QWheelEvent* we = dynamic_cast<QWheelEvent*>(e);
    if (we->modifiers() == Qt::CTRL)
    {
      int numSteps = we->delta()/8/15;//Refer to QWheelEvent doc.
      m_threshold->setUpperThreshold(m_threshold->upperThreshold() + numSteps);//Using stepBy highlight the input text
      m_threshold->setLowerThreshold(m_threshold->lowerThreshold() + numSteps);//Using stepBy highlight the input text
      //       if (m_preview)
      //       {
        //         m_preview->setThreshold(m_threshold->threshold());
      //       }
      //       view->view()->forceRender();

      return true;
    }
  }else if(e->type() == QEvent::MouseMove)
  {
    QMouseEvent *me = dynamic_cast<QMouseEvent*>(e);
    if (me->modifiers() == Qt::SHIFT)
    {
      DisplayRegionList regions;
      DisplayRegion singlePixel;

      int xPos, yPos;
      view->eventPosition(xPos, yPos);
      singlePixel << QPoint(xPos,yPos);
      regions << singlePixel;

      PickList pickList = view->pick(m_filters, regions);
      if (pickList.size() == 0)
        return false;

//       Q_ASSERT(pickList.size() == 1);// Only one element selected
//       IPicker::PickedItem element = pickList.first();
//       Q_ASSERT(element.first.size() == 1); // with one pixel
// 
//       QVector3D pick = element.first.first();
//       SelectableItem *input = element.second;
//       int seed[3] = {pick.x(), pick.y(), pick.z()};
      //       if (null == m_preview)
      //       {
        // //     const int w = 40;
      // //       int voi[6] = {seed[0] - w, seed[0] + w,
      // //       seed[1] - w, seed[1] + w,
      // //       seed[2] - w, seed[2] + w};
      //
      //         int voi[6];
      //        view->previewextent(voi);
      //        m_preview = new seedgrowsegmentationfilter(input->volume(), seed, m_threshold->threshold(), voi);
      //        view->addpreview(m_preview);
      //       }
      //       else
      //       {
        //      m_preview->setinput(input->volume());
      //        m_preview->setseed(seed);
      //       }
      //       view->view()->forcerender();
    }else
    {
      //       if (m_preview)
      //       {
        // 	view->removePreview(m_preview);
        // 	delete m_preview;
        // 	m_preview = NULL;
        // 	view->view()->forceRender();
        //       }
    }
  }else if(e->type() == QEvent::MouseButtonPress)
  {
    QMouseEvent *me = dynamic_cast<QMouseEvent*>(e);
    if (me->modifiers() != Qt::CTRL && m_succesor)
      return m_succesor->filterEvent(e,view);
  }

  return false;
}

//-----------------------------------------------------------------------------
QCursor SeedGrowSelector::cursor()
{
  if (m_succesor)
    return m_succesor->cursor();
  else
    return IPicker::cursor();
}

//-----------------------------------------------------------------------------
void SeedGrowSelector::previewOn()
{
  
}

//-----------------------------------------------------------------------------
void SeedGrowSelector::previewOff()
{

}
