/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2011  Jorge Peña <jorge.pena.pastor@gmail.com>

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

#include "SegmentationInspector.h"

#include <QFileDialog>
#include <QDebug>

#include <model/ModelItem.h>
#include <model/Segmentation.h>
#include <model/Filter.h>
#include <gui/VolumeView.h>
#include <EspinaCore.h>

QMap<Segmentation *, SegmentationInspector *> SegmentationInspector::m_inspectors;

//------------------------------------------------------------------------
SegmentationInspector* SegmentationInspector::CreateInspector(Segmentation* seg)
{
  SegmentationInspector *inspector;

  if (m_inspectors.contains(seg))
    inspector = m_inspectors[seg];
  else
    inspector = new SegmentationInspector(seg);

  return inspector;
}


//------------------------------------------------------------------------
SegmentationInspector::SegmentationInspector(Segmentation *seg, QWidget* parent, Qt::WindowFlags f)
: QWidget(parent, f)
, m_seg(seg)
, m_model(EspinaCore::instance()->model())
, m_info(new InformationProxy())
, m_sort(new QSortFilterProxyModel())
{
  setupUi(this);

  m_view->addSegmentationRepresentation(seg);
  m_view->resetCamera();
  m_view->forceRender();

  connect(seg, SIGNAL(modified(ModelItem*)),
	  this, SLOT(updateScene()));

  Filter *filter = seg->filter();
  Q_ASSERT(filter);
  QWidget *widget = filter->createConfigurationWidget();
  m_filterInspector->setWidget(widget);
  m_filterInspector->setMinimumWidth(widget->minimumSize().width());;

  m_info->setQuery(seg->availableInformations());
  m_info->setSourceModel(m_model.data());
  m_sort->setSourceModel(m_info.data());
  m_sort->setFilterRegExp(seg->data().toString());
  m_sort->setDynamicSortFilter(true);

  m_dataView->setModel(m_sort.data());
  m_dataView->setSortingEnabled(true);// Needed to update values when segmentation is modified
  m_dataView->sortByColumn(0, Qt::AscendingOrder);

  m_inspectors[m_seg] = this;
}

//------------------------------------------------------------------------
void SegmentationInspector::closeEvent(QCloseEvent *e)
{
  QWidget::closeEvent(e);
  deleteLater();
}

//------------------------------------------------------------------------
SegmentationInspector::~SegmentationInspector()
{
  m_inspectors.remove(m_seg);
}

//
void SegmentationInspector::updateScene()
{
  m_view->updateSegmentationRepresentation(m_seg);
  m_view->forceRender();
}