/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2011  Jorge Peña Pastor <jpena@cesvima.upm.es>

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


#include "SegmentationExplorer.h"

#include "common/model/EspinaModel.h"
#include "common/model/ModelTest.h"
#include "common/model/proxies/SampleProxy.h"
#include "common/model/proxies/TaxonomyProxy.h"

#include <iostream>
#include <cstdio>

#include <QStringListModel>
#include <EspinaCore.h>
#include <gui/EspinaView.h>
#include <undo/RemoveSegmentation.h>
#include <QMessageBox>

//------------------------------------------------------------------------
class SegmentationExplorer::GUI
: public QWidget
, public Ui::SegmentationExplorer
{
public:
  GUI();
};

SegmentationExplorer::GUI::GUI()
{
  setupUi(this);
}


//------------------------------------------------------------------------
class SegmentationExplorer::Layout
{
public:
  explicit Layout(QSharedPointer<EspinaModel> model): m_model(model) {}
  virtual ~Layout(){}

  virtual QAbstractItemModel *model() {return m_model.data();}
  virtual void deleteSegmentation(QModelIndexList indices) {};

protected:
  QSharedPointer<EspinaModel> m_model;
};

//------------------------------------------------------------------------
class SampleLayout : public SegmentationExplorer::Layout
{
public:
  explicit SampleLayout(QSharedPointer<EspinaModel> model);
  virtual ~SampleLayout(){}

  virtual QAbstractItemModel* model() {return m_proxy.data();}
  virtual void deleteSegmentation(QModelIndexList indices);

private:
  QSharedPointer<SampleProxy> m_proxy;
};

//------------------------------------------------------------------------
SampleLayout::SampleLayout(QSharedPointer<EspinaModel> model)
: Layout(model)
, m_proxy(new SampleProxy())
{
  m_proxy->setSourceModel(m_model.data());
}

//------------------------------------------------------------------------
void SampleLayout::deleteSegmentation(QModelIndexList indices)
{
  QSet<Segmentation *> toDelete;
  foreach(QModelIndex index, indices)
  {
    ModelItem *item = indexPtr(index);
    switch (item->type())
    {
      case ModelItem::SEGMENTATION:
      {
	Segmentation *seg = dynamic_cast<Segmentation *>(item);
	Q_ASSERT(seg);
	toDelete << seg;
	break;
      }
      case ModelItem::SAMPLE:
      {
	int totalSeg = m_proxy->numSegmentations(index, true);
	int directSeg = m_proxy->numSegmentations(index);

	if (totalSeg == 0)
	  continue;
	
	Sample *sample = dynamic_cast<Sample *>(item);
	QMessageBox msgBox;
	msgBox.setText(QString("Delete %1's segmentations").arg(sample->id()));
	msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
	msgBox.setDefaultButton(QMessageBox::No);

	if (directSeg > 0)
	{
	  if (directSeg < totalSeg)
	  {
	    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::YesAll |  QMessageBox::No);
	    msgBox.setText(QString("Delete %1's segmentations. If you want to delete recursively select Yes To All").arg(sample->id()));
	  }
	} else
	{
	  msgBox.setText(QString("Delete recursively %1's segmentations").arg(sample->id()));
	  msgBox.setStandardButtons(QMessageBox::YesAll |  QMessageBox::No);
	}

	bool recursive = false;
	switch (msgBox.exec())
	{
	  case QMessageBox::YesAll:
	    recursive = true;
	  case QMessageBox::Yes:
	  {
	    QModelIndexList subSegs = m_proxy->segmentations(index, recursive);
	    foreach(QModelIndex subIndex, subSegs)
	    {
	      ModelItem *subItem = indexPtr(subIndex);
	      Segmentation *seg = dynamic_cast<Segmentation *>(subItem);
	      Q_ASSERT(seg);
	      toDelete << seg;
	    }
	    break;
	  }
	  default:
	    break;
	}
	break;
      }
      default:
	Q_ASSERT(false);
    }
  }

  if (!toDelete.isEmpty())
  {
  QSharedPointer<QUndoStack> undoStack = EspinaCore::instance()->undoStack();
  undoStack->beginMacro("Delete Segmentations");
  undoStack->push(new RemoveSegmentation(toDelete.toList()));
  undoStack->endMacro();
  }

}

//------------------------------------------------------------------------
class TaxonomyLayout : public SegmentationExplorer::Layout
{
public:
  explicit TaxonomyLayout(QSharedPointer<EspinaModel> model);
  virtual ~TaxonomyLayout(){}

  virtual QAbstractItemModel* model() {return m_proxy.data();}
  virtual void deleteSegmentation(QModelIndexList indices);

private:
  QSharedPointer<TaxonomyProxy> m_proxy;
};

//------------------------------------------------------------------------
TaxonomyLayout::TaxonomyLayout(QSharedPointer<EspinaModel> model)
: Layout(model)
, m_proxy(new TaxonomyProxy())
{
  m_proxy->setSourceModel(m_model.data());
}

//------------------------------------------------------------------------
void TaxonomyLayout::deleteSegmentation(QModelIndexList indices)
{
  QSet<Segmentation *> toDelete;
  foreach(QModelIndex index, indices)
  {
    ModelItem *item = indexPtr(index);
    switch (item->type())
    {
      case ModelItem::SEGMENTATION:
      {
	Segmentation *seg = dynamic_cast<Segmentation *>(item);
	Q_ASSERT(seg);
	toDelete << seg;
	break;
      }
      case ModelItem::TAXONOMY:
      {
	int totalSeg = m_proxy->numSegmentations(index, true);
	int directSeg = m_proxy->numSegmentations(index);

	if (totalSeg == 0)
	  continue;
	
	TaxonomyNode *taxonmy = dynamic_cast<TaxonomyNode *>(item);
	QMessageBox msgBox;
	msgBox.setText(QString("Delete %1's segmentations").arg(taxonmy->qualifiedName()));
	msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
	msgBox.setDefaultButton(QMessageBox::No);

	if (directSeg > 0)
	{
	  if (directSeg < totalSeg)
	  {
	    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::YesAll |  QMessageBox::No);
	    msgBox.setText(QString("Delete %1's segmentations. If you want to delete recursively select Yes To All").arg(taxonmy->qualifiedName()));
	  }
	} else
	{
	  msgBox.setText(QString("Delete recursively %1's segmentations").arg(taxonmy->qualifiedName()));
	  msgBox.setStandardButtons(QMessageBox::YesAll |  QMessageBox::No);
	}
	
	bool recursive = false;
	switch (msgBox.exec())
	{
	  case QMessageBox::YesAll:
	    recursive = true;
	  case QMessageBox::Yes:
	  {
	    QModelIndexList subSegs = m_proxy->segmentations(index, recursive);
	    foreach(QModelIndex subIndex, subSegs)
	    {
	      ModelItem *subItem = indexPtr(subIndex);
	      Segmentation *seg = dynamic_cast<Segmentation *>(subItem);
	      Q_ASSERT(seg);
	      toDelete << seg;
	    }
	    break;
	  }
	  default:
	    break;
	}
	break;
      }
      default:
	Q_ASSERT(false);
    }
  }

  if (!toDelete.isEmpty())
  {
  QSharedPointer<QUndoStack> undoStack = EspinaCore::instance()->undoStack();
  undoStack->beginMacro("Delete Segmentations");
  undoStack->push(new RemoveSegmentation(toDelete.toList()));
  undoStack->endMacro();
  }
}


//------------------------------------------------------------------------
SegmentationExplorer::SegmentationExplorer(QSharedPointer< EspinaModel> model, QWidget* parent)
: EspinaDockWidget(parent)
, m_gui(new GUI())
, m_baseModel(model)
, m_layout(NULL)
{
  setWindowTitle(tr("Segmentation Explorer"));
  setObjectName("SegmentationExplorer");

//   addLayout("Debug", new Layout(m_baseModel));
  addLayout("Taxonomy", new TaxonomyLayout(m_baseModel));
  addLayout("Location", new SampleLayout  (m_baseModel));

  QStringListModel *layoutModel = new QStringListModel(m_layoutNames);
  m_gui->groupList->setModel(layoutModel);
  changeLayout(0);

  connect(m_gui->groupList, SIGNAL(currentIndexChanged(int)),
	  this, SLOT(changeLayout(int)));
  connect(m_gui->view, SIGNAL(doubleClicked(QModelIndex)),
	  this, SLOT(focusOnSegmentation(QModelIndex)));
  connect(m_gui->deleteSegmentation, SIGNAL(clicked(bool)),
          this, SLOT(deleteSegmentation()));
  connect(m_gui->view->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
	  this, SLOT(updateSelection(QItemSelection, QItemSelection)));

  setWidget(m_gui);
}

//------------------------------------------------------------------------
SegmentationExplorer::~SegmentationExplorer()
{
}

//------------------------------------------------------------------------
void SegmentationExplorer::addLayout(const QString id, SegmentationExplorer::Layout* proxy)
{
  m_layoutNames << id;
  m_layouts << proxy;
}

//------------------------------------------------------------------------
void SegmentationExplorer::changeLayout(int index)
{
  Q_ASSERT(index < m_layouts.size());
  if (m_layout)
  {
    disconnect(m_layout->model(), SIGNAL(dataChanged(QModelIndex,QModelIndex)),
	    this, SLOT(updateSelection(QModelIndex)));
  }
  m_layout = m_layouts[index];
#ifdef DEBUG
  m_modelTester = QSharedPointer<ModelTest>(new ModelTest(m_layout->model()));
#endif
  m_gui->view->setModel(m_layout->model());
  connect(m_layout->model(), SIGNAL(dataChanged(QModelIndex,QModelIndex)),
	  this, SLOT(updateSelection(QModelIndex)));
}

//------------------------------------------------------------------------
void SegmentationExplorer::focusOnSegmentation(const QModelIndex& index)
{
  ModelItem *item = indexPtr(index);

  if (ModelItem::SEGMENTATION != item->type())
    return;

  Segmentation *seg = dynamic_cast<Segmentation *>(item);
  Q_ASSERT(seg);
  EspinaView *view = EspinaCore::instance()->viewManger()->currentView();
  int x = seg->information("Centroid X").toInt();
  int y = seg->information("Centroid Y").toInt();
  int z = seg->information("Centroid Z").toInt();
  view->setCenter(x, y, z);
}

//------------------------------------------------------------------------
void SegmentationExplorer::deleteSegmentation()
{
  if (m_layout)
    m_layout->deleteSegmentation(m_gui->view->selectionModel()->selectedIndexes());
}

//------------------------------------------------------------------------
void SegmentationExplorer::updateSelection(QModelIndex index)
{
  if (index.isValid())
  {
    ModelItem *item = indexPtr(index);
    if (ModelItem::SEGMENTATION == item->type())
    {
      m_gui->view->blockSignals(true);
      Segmentation *seg = dynamic_cast<Segmentation *>(item);
      if (seg->selected())
	m_gui->view->selectionModel()->setCurrentIndex(index, QItemSelectionModel::Select);
      else
	m_gui->view->selectionModel()->select(index, QItemSelectionModel::Deselect);
      m_gui->view->blockSignals(false);
    }
  }
}

//------------------------------------------------------------------------
void SegmentationExplorer::updateSelection(QItemSelection selected, QItemSelection deselected)
{
  m_layout->model()->blockSignals(true);
  foreach(QModelIndex index, selected.indexes())
    m_layout->model()->setData(index, true, Segmentation::SelectionRole);

  foreach(QModelIndex index, deselected.indexes())
    m_layout->model()->setData(index, false, Segmentation::SelectionRole);
  m_layout->model()->blockSignals(false);
}
