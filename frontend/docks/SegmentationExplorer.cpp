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

#ifdef TEST_ESPINA_MODELS
#include "common/model/ModelTest.h"
#endif

#include "common/EspinaCore.h"
#include <gui/EspinaView.h>
#include <undo/RemoveSegmentation.h>

#include "SegmentationInspector.h"
#include "SegmentationDelegate.h"
#include "common/model/proxies/SampleProxy.h"
#include "common/model/proxies/TaxonomyProxy.h"
#include <model/Segmentation.h>
#include <selection/SelectionManager.h>

#include <iostream>
#include <cstdio>

#include <QStringListModel>
#include <QMessageBox>
#include <QSortFilterProxyModel>

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
  view->setSortingEnabled(true);
  view->sortByColumn(0, Qt::AscendingOrder);

  showInformation->setIcon(
    qApp->style()->standardIcon(QStyle::SP_MessageBoxInformation));
}


//------------------------------------------------------------------------
class SegmentationExplorer::Layout
{
public:
  explicit Layout(QSharedPointer<EspinaModel> model): m_model(model) {}
  virtual ~Layout(){}

  virtual QAbstractItemModel *model() {return m_model.data();}
  virtual ModelItem *item(const QModelIndex &index) const {return indexPtr(index);}
  virtual QModelIndex index(ModelItem *item) const
  { return m_model->index(item); }
  virtual void deleteSegmentation(QModelIndexList indices) {};

protected:
  QSharedPointer<EspinaModel> m_model;
};

bool sortSegmentationLessThan(ModelItem *left, ModelItem *right)
{
  Segmentation *leftSeg = dynamic_cast<Segmentation *>(left);
  Segmentation *rightSeg = dynamic_cast<Segmentation *>(right);

  if (leftSeg->number() == rightSeg->number())
    return left->data(Qt::ToolTipRole).toString() <
           right->data(Qt::ToolTipRole).toString();
  else
    return leftSeg->number() < rightSeg->number();
}

//------------------------------------------------------------------------
class SampleLayout : public SegmentationExplorer::Layout
{
  class SampleSortFilter : public QSortFilterProxyModel
  {
  protected:
    virtual bool lessThan(const QModelIndex& left, const QModelIndex& right) const
    {
      ModelItem *leftItem = indexPtr(left);
      ModelItem *rightItem = indexPtr(right);
      if (leftItem->type() == rightItem->type())
	if (ModelItem::SEGMENTATION == leftItem->type())
	  return sortSegmentationLessThan(leftItem, rightItem);
	else
	  return leftItem->data(Qt::DisplayRole).toString() < rightItem->data(Qt::DisplayRole).toString();
      else
	return leftItem->type() == ModelItem::TAXONOMY;
    }
  };

public:
  explicit SampleLayout(QSharedPointer<EspinaModel> model);
  virtual ~SampleLayout(){}

  virtual QAbstractItemModel* model() {return m_sort.data();}
  virtual ModelItem* item(const QModelIndex& index) const
  { return indexPtr(m_sort->mapToSource(index)); }
  virtual QModelIndex index(ModelItem* item) const
  { return m_sort->mapFromSource(m_proxy->mapFromSource(Layout::index(item))); }
  virtual void deleteSegmentation(QModelIndexList indices);

private:
  QSharedPointer<SampleProxy> m_proxy;
  QSharedPointer<SampleSortFilter> m_sort;
};

//------------------------------------------------------------------------
SampleLayout::SampleLayout(QSharedPointer<EspinaModel> model)
: Layout(model)
, m_proxy(new SampleProxy())
, m_sort (new SampleSortFilter())
{
  m_proxy->setSourceModel(m_model.data());
  m_sort->setSourceModel(m_proxy.data());
  m_sort->setDynamicSortFilter(true);
}

//------------------------------------------------------------------------
void SampleLayout::deleteSegmentation(QModelIndexList indices)
{
  QSet<Segmentation *> toDelete;
  foreach(QModelIndex index, indices)
  {
    index = m_sort->mapToSource(index);
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
        int totalSeg  = m_proxy->numSegmentations(index, true);
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
  class TaxonomySortFilter : public QSortFilterProxyModel
  {
  protected:
    virtual bool lessThan(const QModelIndex& left, const QModelIndex& right) const
    {
      ModelItem *leftItem = indexPtr(left);
      ModelItem *rightItem = indexPtr(right);
      if (leftItem->type() == rightItem->type())
        if (ModelItem::SEGMENTATION == leftItem->type())
          return sortSegmentationLessThan(leftItem, rightItem);
        else
          return leftItem->data(Qt::DisplayRole).toString() < rightItem->data(Qt::DisplayRole).toString();
        else
          return leftItem->type() == ModelItem::TAXONOMY;
    }
  };
public:
  explicit TaxonomyLayout(QSharedPointer<EspinaModel> model);
  virtual ~TaxonomyLayout(){}

  virtual QAbstractItemModel* model() {return m_sort.data();}
  virtual ModelItem* item(const QModelIndex& index) const
  { return indexPtr(m_sort->mapToSource(index)); }
  virtual QModelIndex index(ModelItem* item) const
  { return m_sort->mapFromSource(m_proxy->mapFromSource(Layout::index(item))); }
  virtual void deleteSegmentation(QModelIndexList indices);

private:
  QSharedPointer<TaxonomyProxy> m_proxy;
  QSharedPointer<TaxonomySortFilter> m_sort;
};

//------------------------------------------------------------------------
TaxonomyLayout::TaxonomyLayout(QSharedPointer<EspinaModel> model)
: Layout(model)
, m_proxy(new TaxonomyProxy())
, m_sort (new TaxonomySortFilter())
{
  m_proxy->setSourceModel(m_model.data());
  m_sort->setSourceModel(m_proxy.data());
  m_sort->setDynamicSortFilter(true);
}

//------------------------------------------------------------------------
void TaxonomyLayout::deleteSegmentation(QModelIndexList indices)
{
  QSet<Segmentation *> toDelete;
  foreach(QModelIndex index, indices)
  {
    index = m_sort->mapToSource(index);
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
SegmentationExplorer::SegmentationExplorer(QSharedPointer<EspinaModel> model,
                                           QWidget* parent)
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
  connect(m_gui->showInformation, SIGNAL(clicked(bool)),
          this, SLOT(showInformation()));
  connect(m_gui->deleteSegmentation, SIGNAL(clicked(bool)),
          this, SLOT(deleteSegmentation()));
  connect(m_gui->view->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
          this, SLOT(updateSelection(QItemSelection, QItemSelection)));
  connect(SelectionManager::instance(), SIGNAL(selectionChanged(SelectionManager::Selection)),
          this, SLOT(updateSelection(SelectionManager::Selection)));

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

//------------------------------------------------------------------------sele
void SegmentationExplorer::changeLayout(int index)
{
  Q_ASSERT(index < m_layouts.size());
  m_layout = m_layouts[index];
#ifdef TEST_ESPINA_MODELS
  m_modelTester = QSharedPointer<ModelTest>(new ModelTest(m_layout->model()));
#endif
  m_gui->view->setModel(m_layout->model());
  m_gui->view->setItemDelegate(new SegmentationDelegate());
}

//------------------------------------------------------------------------
void SegmentationExplorer::focusOnSegmentation(const QModelIndex& index)
{
  ModelItem *item = m_layout->item(index);

  if (ModelItem::SEGMENTATION != item->type())
    return;

  const Nm *p = SelectionManager::instance()->selectionCenter();
  EspinaView *view = EspinaCore::instance()->viewManger()->currentView();
  view->setCrosshairPoint(p[0], p[1], p[2]);
  view->setCameraFocus(p);
//   Q_ASSERT(seg);
//   x = seg->information("Centroid X").toInt();
//   y = seg->information("Centroid Y").toInt();
//   z = seg->information("Centroid Z").toInt();
//   view->setCenter(x, y, z);
}

//------------------------------------------------------------------------
void SegmentationExplorer::showInformation()
{
  foreach(QModelIndex index, m_gui->view->selectionModel()->selectedIndexes())
  {
    ModelItem *item = m_layout->item(index);
    Q_ASSERT(item);

    if (ModelItem::SEGMENTATION == item->type())
    {
      Segmentation *seg = dynamic_cast<Segmentation *>(item);
      SegmentationInspector *inspector = SegmentationInspector::CreateInspector(seg);
      inspector->show();
      inspector->raise();
    }
  }
}

//------------------------------------------------------------------------
void SegmentationExplorer::deleteSegmentation()
{
  if (m_layout)
    m_layout->deleteSegmentation(m_gui->view->selectionModel()->selectedIndexes());
}

//------------------------------------------------------------------------
void SegmentationExplorer::updateSelection(SelectionManager::Selection selection)
{
//   qDebug() << "Update Seg Explorer Selection from Selection Manager";
  m_gui->view->blockSignals(true);
  m_gui->view->selectionModel()->blockSignals(true);
  m_gui->view->selectionModel()->reset();
  foreach(SelectableItem *item, selection)
  {
    QModelIndex index = m_layout->index(item);
    if (index.isValid())
      m_gui->view->selectionModel()->select(index, QItemSelectionModel::Select);
  }
  m_gui->view->selectionModel()->blockSignals(false);
  m_gui->view->blockSignals(false);
  // Center the view at the first selected item
  if (!selection.isEmpty())
  {
    QModelIndex currentIndex = m_layout->index(selection.first());
    m_gui->view->selectionModel()->setCurrentIndex(currentIndex, QItemSelectionModel::Select);
  }
  // Update all visible items
  m_gui->view->viewport()->update();
}

//------------------------------------------------------------------------
void SegmentationExplorer::updateSelection(QItemSelection selected, QItemSelection deselected)
{
  SelectionManager::Selection selection;

  foreach(QModelIndex index, m_gui->view->selectionModel()->selection().indexes())
  {
    ModelItem *item = m_layout->item(index);
    if (ModelItem::SEGMENTATION == item->type())
      selection << dynamic_cast<SelectableItem *>(item);
  }

  SelectionManager::instance()->setSelection(selection);
}
