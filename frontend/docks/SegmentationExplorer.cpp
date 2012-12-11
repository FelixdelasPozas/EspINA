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


// EspINA
#include "common/model/EspinaModel.h"
#include "common/model/Sample.h"
#include "common/model/Segmentation.h"
#include "common/model/proxies/SampleProxy.h"
#include "common/model/proxies/TaxonomyProxy.h"
#include "common/settings/ISettingsPanel.h"
#include "common/undo/RemoveSegmentation.h"
#include "frontend/docks/SegmentationInspector.h"
#include "frontend/docks/SegmentationDelegate.h"
#include "common/EspinaRegions.h"

#ifdef TEST_ESPINA_MODELS
#include "common/model/ModelTest.h"
#endif

// Qt
#include <QMessageBox>
#include <QSortFilterProxyModel>
#include <QStringListModel>
#include <QUndoStack>

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

const QString SEGMENTATION_MESSAGE = QObject::tr("Delete %1's segmentations");
const QString RECURSIVE_MESSAGE = QObject::tr("Delete %1's segmentations. "
                                              "If you want to delete recursively select Yes To All");
const QString MIXED_MESSAGE = QObject::tr("Delete recursively %1's segmentations");

//------------------------------------------------------------------------
class SegmentationExplorer::Layout
{
public:
  explicit Layout(EspinaModel *model): m_model(model) {}
  virtual ~Layout(){}

  virtual QAbstractItemModel *model()
  {return m_model; }
  virtual ModelItem *item(const QModelIndex &index) const {return indexPtr(index);}
  virtual QModelIndex index(ModelItem *item) const
  { return m_model->index(item); }
  virtual SegmentationList deletedSegmentations(QModelIndexList indices)
  { return SegmentationList(); }

protected:
  EspinaModel *m_model;
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
class SampleLayout
: public SegmentationExplorer::Layout
{
  class SampleSortFilter
  : public QSortFilterProxyModel
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
  explicit SampleLayout(EspinaModel *model);
  virtual ~SampleLayout(){}

  virtual QAbstractItemModel* model() {return m_sort.data();}
  virtual ModelItem* item(const QModelIndex& index) const
  { return indexPtr(m_sort->mapToSource(index)); }
  virtual QModelIndex index(ModelItem* item) const
  { return m_sort->mapFromSource(m_proxy->mapFromSource(Layout::index(item))); }
  virtual SegmentationList deletedSegmentations(QModelIndexList indices);

private:
  QSharedPointer<SampleProxy> m_proxy;
  QSharedPointer<SampleSortFilter> m_sort;
};

//------------------------------------------------------------------------
SampleLayout::SampleLayout(EspinaModel *model)
: Layout(model)
, m_proxy(new SampleProxy())
, m_sort (new SampleSortFilter())
{
  m_proxy->setSourceModel(m_model);
  m_sort->setSourceModel(m_proxy.data());
  m_sort->setDynamicSortFilter(true);
}

//------------------------------------------------------------------------
SegmentationList SampleLayout::deletedSegmentations(QModelIndexList indices)
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
        msgBox.setText(SEGMENTATION_MESSAGE.arg(sample->id()));
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::No);

        if (directSeg > 0)
        {
          if (directSeg < totalSeg)
          {
            msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::YesAll |  QMessageBox::No);
            msgBox.setText(MIXED_MESSAGE.arg(sample->id()));
          }
        } else
        {
          msgBox.setText(RECURSIVE_MESSAGE.arg(sample->id()));
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
            break;
    }
  }

  return toDelete.toList();
}

//------------------------------------------------------------------------
class TaxonomyLayout
: public SegmentationExplorer::Layout
{
  class TaxonomySortFilter
  : public QSortFilterProxyModel
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
  explicit TaxonomyLayout(EspinaModel *model);
  virtual ~TaxonomyLayout(){}

  virtual QAbstractItemModel* model()
  { return m_sort.data(); }
  virtual ModelItem* item(const QModelIndex& index) const
  { return indexPtr(m_sort->mapToSource(index)); }
  virtual QModelIndex index(ModelItem* item) const
  { return m_sort->mapFromSource(m_proxy->mapFromSource(Layout::index(item))); }
  virtual SegmentationList deletedSegmentations(QModelIndexList indices);

private:
  QSharedPointer<TaxonomyProxy> m_proxy;
  QSharedPointer<TaxonomySortFilter> m_sort;
};

//------------------------------------------------------------------------
TaxonomyLayout::TaxonomyLayout(EspinaModel *model)
: Layout(model)
, m_proxy(new TaxonomyProxy())
, m_sort (new TaxonomySortFilter())
{
  m_proxy->setSourceModel(m_model);
  m_sort->setSourceModel(m_proxy.data());
  m_sort->setDynamicSortFilter(true);
}

//------------------------------------------------------------------------
SegmentationList TaxonomyLayout::deletedSegmentations(QModelIndexList indices)
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

        TaxonomyElement *taxonmy = dynamic_cast<TaxonomyElement *>(item);
        QMessageBox msgBox;
        msgBox.setText(SEGMENTATION_MESSAGE.arg(taxonmy->qualifiedName()));
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::No);

        if (directSeg > 0)
        {
          if (directSeg < totalSeg)
          {
            msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::YesAll |  QMessageBox::No);
            msgBox.setText(MIXED_MESSAGE.arg(taxonmy->qualifiedName()));
          }
        } else
        {
          msgBox.setText(RECURSIVE_MESSAGE.arg(taxonmy->qualifiedName()));
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
            break;
    }
  }

  return toDelete.toList();
}


//------------------------------------------------------------------------
SegmentationExplorer::SegmentationExplorer(EspinaModel *model,
                                           QUndoStack  *undoStack,
                                           ViewManager *vm,
                                           QWidget* parent)
: QDockWidget(parent)
, m_gui(new GUI())
, m_baseModel(model)
, m_undoStack(undoStack)
, m_viewManager(vm)
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
          this, SLOT(deleteSegmentations()));
  connect(m_gui->view->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
          this, SLOT(updateSelection(QItemSelection, QItemSelection)));
  connect(m_viewManager, SIGNAL(selectionChanged(ViewManager::Selection)),
          this, SLOT(updateSelection(ViewManager::Selection)));
  connect(m_baseModel, SIGNAL(rowsAboutToBeRemoved(QModelIndex, int , int)),
          this, SLOT(segmentationsDeleted(QModelIndex,int,int)));

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
  m_layout = m_layouts[index];
#ifdef TEST_ESPINA_MODELS
  m_modelTester = QSharedPointer<ModelTest>(new ModelTest(m_layout->model()));
#endif
  m_gui->view->setModel(m_layout->model());
  m_gui->view->setItemDelegate(new SegmentationDelegate(m_baseModel, m_undoStack, m_viewManager)); //TODO 2012-10-05 Sigue sirviendo para algo??
}

//------------------------------------------------------------------------
void SegmentationExplorer::focusOnSegmentation(const QModelIndex& index)
{
  ModelItem *item = m_layout->item(index);

  if (ModelItem::SEGMENTATION != item->type())
    return;

  Nm bounds[6];
  Segmentation *seg = dynamic_cast<Segmentation*>(item);
  VolumeBounds(seg->itkVolume(), bounds);
  Nm center[3] = { (bounds[0] + bounds[1])/2, (bounds[2] + bounds[3])/2, (bounds[4] + bounds[5])/2 };
  m_viewManager->focusViewsOn(center);

  /* TODO BUG 2012-10-05 Use "center on" selection
  const Nm *p = SelectionManager::instance()->selectionCenter();
  EspinaView *view = EspinaCore::instance()->viewManger()->currentView();
  view->setCrosshairPoint(p[0], p[1], p[2]);
  view->setCameraFocus(p);                     cbbp
  */
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
      SegmentationInspector *inspector = m_inspectors.value(seg, NULL);
      if (!inspector)
      {
        inspector = new SegmentationInspector(seg, m_baseModel, m_undoStack, m_viewManager);
        connect(inspector, SIGNAL(inspectorClosed(SegmentationInspector*)),
                this, SLOT(releaseInspectorResources(SegmentationInspector*)));
        m_inspectors[seg] = inspector;
      }
      inspector->show();
      inspector->raise();
    }
  }
}

//------------------------------------------------------------------------
void SegmentationExplorer::segmentationsDeleted(const QModelIndex parent, int start, int end)
{
  if (m_baseModel->segmentationRoot() == parent)
  {
    for(int row = start; row <= end; row++)
    {
      QModelIndex child = parent.child(row, 0);
      ModelItem *item = indexPtr(child);
      Segmentation *seg = dynamic_cast<Segmentation *>(item);
      Q_ASSERT(seg);
      SegmentationInspector *inspector = m_inspectors.value(seg, NULL);
      if (inspector)
      {
        m_inspectors.remove(seg);
        inspector->hide();
        delete inspector;
      }
      }
  }
}

//------------------------------------------------------------------------
void SegmentationExplorer::deleteSegmentations()
{
  if (m_layout)
  {
    QModelIndexList selected = m_gui->view->selectionModel()->selectedIndexes();
    SegmentationList toDelete = m_layout->deletedSegmentations(selected);
    if (!toDelete.isEmpty())
    {
      m_undoStack->beginMacro("Delete Segmentations");
      m_undoStack->push(new RemoveSegmentation(toDelete, m_baseModel));
      m_undoStack->endMacro();
    }
  }
}

//------------------------------------------------------------------------
void SegmentationExplorer::updateSelection(ViewManager::Selection selection)
{
//   qDebug() << "Update Seg Explorer Selection from Selection Manager";
  m_gui->view->blockSignals(true);
  m_gui->view->selectionModel()->blockSignals(true);
  m_gui->view->selectionModel()->reset();
  foreach(PickableItem *item, selection)
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
    m_gui->view->scrollTo(currentIndex);
  }
  // Update all visible items
  m_gui->view->viewport()->update();
}

//------------------------------------------------------------------------
void SegmentationExplorer::updateSelection(QItemSelection selected, QItemSelection deselected)
{
  ViewManager::Selection selection;

  foreach(QModelIndex index, m_gui->view->selectionModel()->selection().indexes())
  {
    ModelItem *item = m_layout->item(index);
    if (ModelItem::SEGMENTATION == item->type())
      selection << dynamic_cast<PickableItem *>(item);
  }

  m_viewManager->setSelection(selection);
}

//------------------------------------------------------------------------
void SegmentationExplorer::releaseInspectorResources(SegmentationInspector* inspector)
{
  foreach(Segmentation *seg, m_inspectors.keys())
  {
    if (m_inspectors[seg] == inspector)
    {
      m_inspectors.remove(seg);
      delete inspector;

      return;
    }
  }
}

//------------------------------------------------------------------------
ISettingsPanel *SegmentationExplorer::settingsPanel()
{
  Q_ASSERT(false); //TODO Check if NULL is correct
  return NULL;
}

//------------------------------------------------------------------------
void SegmentationExplorer::updateSegmentationRepresentations()
{

}

//------------------------------------------------------------------------
void SegmentationExplorer::updateSelection()
{
  std::cout << "update selection\n" << std::flush;
}
