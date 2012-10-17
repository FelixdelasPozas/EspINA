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


#include "RectangularVOI.h"
#include "common/model/Channel.h"
#include <gui/ViewManager.h>

#include <QPixmap>
#include <boost/concept_check.hpp>

//-----------------------------------------------------------------------------
RectangularVOI::RectangularVOI(ViewManager *vm)
: m_viewManager(vm)
, m_enabled(true)
, m_interactive(true)
, m_widget (NULL)
{
  m_picker.setCursor(QCursor(QPixmap(":roi_go.svg").scaled(32,32)));
  m_picker.setMultiSelection(false);
  m_picker.setPickable(IPicker::CHANNEL);
  connect(&m_picker, SIGNAL(itemsPicked(IPicker::PickList)),
          this, SLOT(defineVOI(IPicker::PickList)));
}

//-----------------------------------------------------------------------------
RectangularVOI::~RectangularVOI()
{

}

//-----------------------------------------------------------------------------
QCursor RectangularVOI::cursor() const
{
  QCursor voiCursor(Qt::ArrowCursor);

  if (!m_widget)
    voiCursor= m_picker.cursor();

  return voiCursor;
}


//-----------------------------------------------------------------------------
bool RectangularVOI::filterEvent(QEvent* e, EspinaRenderView* view)
{
  bool eventHandled = false;

  if (m_interactive || true) //TODO 2012-10-17 Until we manage to process vtkWidget's events we have to disable voi's interaction
  {
    if (m_widget)
      eventHandled = m_widget->filterEvent(e,view);

    if (!eventHandled && !m_widget)
      eventHandled = m_picker.filterEvent(e, view);
  }

  return eventHandled;
}

//-----------------------------------------------------------------------------
void RectangularVOI::setEnabled(bool enable)
{
  if (m_enabled == enable)
    return;

  m_enabled = enable;

  if (m_enabled)
    m_interactive = true;
  else if (m_widget)
  {
    m_viewManager->removeWidget(m_widget);
    delete m_widget;
    m_widget = NULL;
    m_viewManager->updateViews();
  }

}

//-----------------------------------------------------------------------------
void RectangularVOI::setInteraction(bool enable)
{
  if (m_interactive == enable)
    return;

  m_interactive = enable;
  // TODO 2012-10-17 Commented to test processing widget events
//   if (m_widget)
//     m_widget->setEnabled(enable);
}

//-----------------------------------------------------------------------------
IVOI::Region RectangularVOI::region()
{
  if (!m_widget)
    return NULL;

  m_widget->bounds(m_bounds);

  return m_bounds;
}

//-----------------------------------------------------------------------------
void RectangularVOI::defineVOI(IPicker::PickList channels)
{
  if (channels.isEmpty())
    return;

  // Compute default bounds
  Q_ASSERT(channels.size() == 1); //Only one element is selected
  IPicker::PickedItem pickedItem = channels.first();

  Q_ASSERT(pickedItem.first.size() == 1); //Only one pixel's selected
  QVector3D pos = pickedItem.first.first();

  PickableItem *pItem = pickedItem.second;
  if (ModelItem::CHANNEL != pItem->type())
    return;

  Channel *pickedChannel = dynamic_cast<Channel *>(pItem);
  double spacing[3];
  pickedChannel->spacing(spacing);

  const Nm HALF_VOXEL = 0.5;
  const Nm XHSIZE = (40 + HALF_VOXEL)*spacing[0];
  const Nm YHSIZE = (40 + HALF_VOXEL)*spacing[1];
  const Nm ZHSIZE = (40 + HALF_VOXEL)*spacing[2];

  Nm bounds[6] = {
     pos.x() - XHSIZE, pos.x() + XHSIZE,
     pos.y() - YHSIZE, pos.y() + YHSIZE,
     pos.z() - ZHSIZE, pos.z() + ZHSIZE};

  m_widget = new RectangularRegion(bounds, m_viewManager);
  Q_ASSERT(m_widget);
  m_viewManager->addWidget(m_widget);

  m_viewManager->updateViews();
}