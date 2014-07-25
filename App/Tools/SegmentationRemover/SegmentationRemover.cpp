/*
    
    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>

    This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
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


#include "SegmentationRemover.h"

#include <GUI/Pickers/PixelSelector.h>
#include <Core/Model/Segmentation.h>

#include <QMouseEvent>

using namespace ESPINA;

//----------------------------------------------------------------------------
SegmentationRemover::SegmentationRemover()
: m_picker(new PixelSelector())
{
  m_picker->setPickable(ISelector::SEGMENTATION);
  connect(m_picker, SIGNAL(itemsPicked(ISelector::PickList)),
          this, SLOT(removeSegmentation(ISelector::PickList)));
}

//----------------------------------------------------------------------------
SegmentationRemover::~SegmentationRemover()
{
  delete m_picker;
}

//----------------------------------------------------------------------------
QCursor SegmentationRemover::cursor() const
{
  return QCursor(Qt::CrossCursor);
}

//----------------------------------------------------------------------------
bool SegmentationRemover::filterEvent(QEvent* e, EspinaRenderView* view)
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
void SegmentationRemover::setInUse(bool enable)
{
  if (!enable)
    emit removalAborted();
}

//----------------------------------------------------------------------------
void SegmentationRemover::setEnabled(bool enable)
{
}

//----------------------------------------------------------------------------
bool SegmentationRemover::enabled() const
{
  return true;
}

//----------------------------------------------------------------------------
void SegmentationRemover::removeSegmentation(ISelector::PickList pickedSeg)
{
  if (pickedSeg.size() != 1)
    return;

  ISelector::PickedItem element = pickedSeg.first();

  PickableItemPtr input = element.second;
  Q_ASSERT(ESPINA::SEGMENTATION == input->type());
  SegmentationPtr seg = segmentationPtr(input);

  emit removeSegmentation(seg);
}