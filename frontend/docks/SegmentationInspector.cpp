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

// EspINA
#include "common/model/ModelItem.h"
#include "common/model/Segmentation.h"
#include "common/model/Filter.h"
#include "common/gui/VolumeView.h"
#include "common/gui/ViewManager.h"

// Qt
#include <QFileDialog>
#include <QDebug>

//------------------------------------------------------------------------
SegmentationInspector::SegmentationInspector(Segmentation *seg,
                                             EspinaModel *model,
                                             QUndoStack *undoStack,
                                             ViewManager *vm,
                                             QWidget* parent,
                                             Qt::WindowFlags f)
: QWidget(parent, f)
, m_undoStack(undoStack)
, m_viewManager(vm)
, m_seg(seg)
, m_model(model)
, m_view(new VolumeView(model->factory(), vm))
, m_info(new InformationProxy())
, m_sort(new QSortFilterProxyModel())
{
  setupUi(this);

  m_view->addSegmentation(seg);
  m_view->resetCamera();
  m_view->updateView();
  horizontalLayout->insertWidget(0, m_view);

  connect(seg, SIGNAL(modified(ModelItem*)),
          this, SLOT(updateScene()));

  Filter *filter = seg->filter();
  Q_ASSERT(filter);
  QWidget *widget = filter->createFilterInspector(m_undoStack, m_viewManager);
  m_filterInspector->setWidget(widget);
  m_filterInspector->setMinimumWidth(widget->minimumSize().width());;

  m_info->setQuery(seg->availableInformations());
  m_info->setSourceModel(m_model);
  m_sort->setSourceModel(m_info.data());
  m_sort->setFilterRegExp("^"+seg->data().toString()+"$");
  m_sort->setDynamicSortFilter(true);

  m_dataView->setModel(m_sort.data());
  m_dataView->setSortingEnabled(true);// Needed to update values when segmentation is modified
  m_dataView->sortByColumn(0, Qt::AscendingOrder);

  this->setWindowTitle(seg->data().toString());
}

//------------------------------------------------------------------------
void SegmentationInspector::closeEvent(QCloseEvent *e)
{
  QWidget::closeEvent(e);
  emit inspectorClosed(this);
}

//------------------------------------------------------------------------
SegmentationInspector::~SegmentationInspector()
{
  delete m_view;
}

//
void SegmentationInspector::updateScene()
{
  m_view->updateSegmentation(m_seg);
  m_view->updateView();
}