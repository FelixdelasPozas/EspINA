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

#include "FilterInspector.h"

#include <Core/Model/Segmentation.h>
#include <GUI/ViewManager.h>

#include <QDebug>

//----------------------------------------------------------------------------
FilterInspector::FilterInspector(QUndoStack *undoStack,
                                 ViewManager* vm,
                                 QWidget* parent)
: QDockWidget(parent)
, m_undoStack(undoStack)
, m_viewManager(vm)
, m_filter(NULL)
, m_seg   (NULL)
{
  setWindowTitle("Filter Inspector");
  setObjectName("Filter Inspector Panel");

  connect(m_viewManager, SIGNAL(selectionChanged(ViewManager::Selection)),
          this, SLOT(updatePannel()));
}

//----------------------------------------------------------------------------
FilterInspector::~FilterInspector()
{
}

//----------------------------------------------------------------------------
void FilterInspector::showEvent(QShowEvent *e)
{
  QWidget::showEvent(e);
  updatePannel();
}

//----------------------------------------------------------------------------
void FilterInspector::updatePannel()
{
  if (!isVisible())
    return;

  Segmentation *seg = NULL;
  bool changeWidget = false;

  SegmentationList selectedSegs;
  foreach (PickableItem *item, m_viewManager->selection())
  {
    if (ModelItem::SEGMENTATION == item->type())
      selectedSegs << dynamic_cast<Segmentation *>(item);
  }

  if (selectedSegs.size() == 1)
    seg = selectedSegs[0];

  // Update if segmentation are different
  if (seg != m_seg)
  {
    if (m_seg)
    {
      disconnect(m_seg, SIGNAL(modified(ModelItem*)),
		 this, SLOT(updatePannel()));
    }

    if (seg)
    {
      connect(seg, SIGNAL(modified(ModelItem*)),
	      this, SLOT(updatePannel()));
    }
    m_seg = seg;
    changeWidget = true;
  } else if (m_filter && m_filter != seg->filter())
  {
    changeWidget = true;
  }

  if (changeWidget)
  {
    QWidget *prevWidget = widget();
    if (prevWidget)
      delete prevWidget;

    if (m_seg)
    {
      m_filter = seg->filter();
      setWidget(m_filter->filterInspector()->createWidget(m_undoStack, m_viewManager));
    } else
    {
      m_filter = NULL;
      setWidget(NULL);
    }
  }
}

