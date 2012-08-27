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

#include <EspinaCore.h>
#include <model/Segmentation.h>

#include <QDebug>
#include <selection/SelectionManager.h>

//----------------------------------------------------------------------------
FilterInspector::FilterInspector(QWidget* parent)
: EspinaDockWidget(parent)
, m_filter(NULL)
, m_seg   (NULL)
{
  setWindowTitle("Filter Inspector");
  setObjectName("Filter Inspector Panel");

  connect(SelectionManager::instance(), SIGNAL(selectionChanged(SelectionManager::Selection)),
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

  SelectionManager::Selection selection = SelectionManager::instance()->selection();
  if (selection.size() == 1)
  {
    SelectableItem *item = selection.first();
    if (ModelItem::SEGMENTATION == selection.first()->type())
      seg = dynamic_cast<Segmentation *>(item);
  }

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
      setWidget(m_filter->createConfigurationWidget());
    } else
    {
      m_filter = NULL;
      setWidget(NULL);
    }
  }
}

