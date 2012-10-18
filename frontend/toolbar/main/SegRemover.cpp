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


#include "SegRemover.h"

#include "common/tools/PixelSelector.h"
#include "common/model/Segmentation.h"

#include <QMouseEvent>

//----------------------------------------------------------------------------
SegRemover::SegRemover()
: m_picker(new PixelSelector())
{
  m_picker->setPickable(IPicker::SEGMENTATION);
  connect(m_picker, SIGNAL(itemsPicked(IPicker::PickList)),
          this, SLOT(removeSegmentation(IPicker::PickList)));
}

//----------------------------------------------------------------------------
SegRemover::~SegRemover()
{
  delete m_picker;
}

//----------------------------------------------------------------------------
QCursor SegRemover::cursor() const
{
  return QCursor(Qt::CrossCursor);
}

//----------------------------------------------------------------------------
bool SegRemover::filterEvent(QEvent* e, EspinaRenderView* view)
{
  if(e->type() == QEvent::MouseButtonPress)
  {
    QMouseEvent *me = dynamic_cast<QMouseEvent*>(e);
    if (me->modifiers() != Qt::CTRL && m_picker)
      return m_picker->filterEvent(e,view);
  }
  return false;
}

//----------------------------------------------------------------------------
void SegRemover::setEnabled(bool enable)
{
  if (!enable)
    emit removalAborted();
}

//----------------------------------------------------------------------------
void SegRemover::setInteraction(bool enable)
{
}

//----------------------------------------------------------------------------
bool SegRemover::interactive() const
{

}

//----------------------------------------------------------------------------
void SegRemover::removeSegmentation(IPicker::PickList pickedSeg)
{
  if (pickedSeg.size() != 1)
    return;

  IPicker::PickedItem element = pickedSeg.first();

  PickableItem *input = element.second;
  Q_ASSERT(ModelItem::SEGMENTATION == input->type());
  Segmentation *seg = dynamic_cast<Segmentation *>(input);
  Q_ASSERT(seg);

  emit removeSegmentation(seg);
}
