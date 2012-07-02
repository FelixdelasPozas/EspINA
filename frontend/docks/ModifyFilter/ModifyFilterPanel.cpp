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


#include "ModifyFilterPanel.h"

#include <EspinaCore.h>
#include <model/Segmentation.h>

#include <QDebug>

//----------------------------------------------------------------------------
ModifyFilterPanel::ModifyFilterPanel(QWidget* parent)
: EspinaDockWidget(parent)
, m_model(EspinaCore::instance()->model())
{
  setWindowTitle("Filter Inspector");
  setObjectName("Filter Inspector Panel");

  connect(m_model.data(), SIGNAL(dataChanged(QModelIndex,QModelIndex)),
	  this, SLOT(showOriginFilter(QModelIndex)));
}

//----------------------------------------------------------------------------
ModifyFilterPanel::~ModifyFilterPanel()
{
}

//----------------------------------------------------------------------------
void ModifyFilterPanel::showOriginFilter(QModelIndex index)
{
  if (!isVisible())
    return;

  if (index.parent() == m_model->segmentationRoot())
  {
    ModelItem *item = indexPtr(index);
    Segmentation *seg = dynamic_cast<Segmentation *>(item);

    if (seg == m_currentSeg)
      return;
    else if (seg->isSelected())
    {
      ModelItem::Vector filters = item->relatedItems(ModelItem::IN, "CreateSegmentation");
      if (filters.size() > 0)
      {
	Filter *filter = dynamic_cast<Filter *>(filters.first());
	Q_ASSERT(filter);
	setWidget(filter->createConfigurationWidget());
	m_currentSeg = seg;
      }else{
	setWidget(NULL);
	m_currentSeg = NULL;
      }
    }
  }
}

