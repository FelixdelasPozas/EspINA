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

#include "Dialogs/SegmentationInspector/SegmentationInspector.h"
#include "Docks/SegmentationExplorer/SegmentationDelegate.h"
#include "Docks/SegmentationExplorer/SegmentationExplorerLayout.h"
#include "LayoutComposition.h"
#include "LayoutSample.h"
#include "LayoutTaxonomy.h"

// EspINA
#include <Core/Model/EspinaModel.h>
#include <Core/Model/Proxies/TaxonomyProxy.h>
#include <Core/Model/Sample.h>
#include <Core/Model/Segmentation.h>
#include <Core/Model/HierarchyItem.h>
#include <GUI/ISettingsPanel.h>
#include <GUI/QtWidget/SegmentationContextualMenu.h>
#include <Undo/RemoveSegmentation.h>

#ifdef TEST_ESPINA_MODELS
#include <Core/Model/ModelTest.h>
#endif

// Qt
#include <QContextMenuEvent>
#include <QMenu>
#include <QSortFilterProxyModel>
#include <QStringListModel>
#include <QUndoStack>
#include <QWidgetAction>

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
SegmentationExplorer::SegmentationExplorer(EspinaModel *model,
                                           QUndoStack  *undoStack,
                                           ViewManager *vm,
                                           QWidget* parent)
: QDockWidget(parent)
, m_gui        (new GUI())
, m_baseModel  (model)
, m_undoStack  (undoStack)
, m_viewManager(vm)
, m_layout     (NULL)
{
  setWindowTitle(tr("Segmentation Explorer"));
  setObjectName("SegmentationExplorer");

  //   addLayout("Debug", new Layout(m_baseModel));
  addLayout("Taxonomy",    new TaxonomyLayout   (m_baseModel));
  addLayout("Composition", new CompositionLayout(m_baseModel));
  addLayout("Location",    new SampleLayout     (m_baseModel));

  QStringListModel *layoutModel = new QStringListModel(m_layoutNames);
  m_gui->groupList->setModel(layoutModel);
  changeLayout(0);

  connect(m_gui->groupList, SIGNAL(currentIndexChanged(int)),
          this, SLOT(changeLayout(int)));
  connect(m_gui->view, SIGNAL(doubleClicked(QModelIndex)),
          this, SLOT(focusOnSegmentation(QModelIndex)));
  connect(m_gui->view, SIGNAL(itemStateChanged(QModelIndex)),
          m_viewManager, SLOT(updateViews()));
  connect(m_gui->showInformation, SIGNAL(clicked(bool)),
          this, SLOT(showInformation()));
  connect(m_gui->deleteSegmentation, SIGNAL(clicked(bool)),
          this, SLOT(deleteSegmentations()));
  connect(m_viewManager, SIGNAL(selectionChanged(ViewManager::Selection, bool)),
          this, SLOT(updateSelection(ViewManager::Selection)));
  connect(m_baseModel, SIGNAL(rowsAboutToBeRemoved(QModelIndex, int , int)),
          this, SLOT(rowsAboutToBeRemoved(QModelIndex, int,int)));

  setWidget(m_gui);

  m_gui->view->installEventFilter(this);
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
bool SegmentationExplorer::eventFilter(QObject *sender, QEvent *e)
{
  if (sender == m_gui->view && QEvent::ContextMenu == e->type())
  {
    QContextMenuEvent *cme = static_cast<QContextMenuEvent *>(e);

    SegmentationContextualMenu contextMenu(m_baseModel, m_viewManager->selectedSegmentations());
    connect(&contextMenu, SIGNAL(deleteSegmentations()),
            this, SLOT(deleteSegmentations()));
    connect(&contextMenu, SIGNAL(changeTaxonomy(TaxonomyElement*)),
            this, SLOT(changeTaxonomy(TaxonomyElement*)));
    connect(&contextMenu, SIGNAL(changeFinalNode(bool)),
            this, SLOT(changeFinalFlag(bool)));

    contextMenu.exec(cme->globalPos());

    return true;
  }

  return QObject::eventFilter(sender, e);
}

//------------------------------------------------------------------------
void SegmentationExplorer::changeLayout(int index)
{
  Q_ASSERT(index < m_layouts.size());
  if (m_layout)
  {
    disconnect(m_gui->view->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
               this, SLOT(updateSelection(QItemSelection, QItemSelection)));
  }

  m_layout = m_layouts[index];
#ifdef TEST_ESPINA_MODELS
  m_modelTester = QSharedPointer<ModelTest>(new ModelTest(m_layout->model()));
#endif
  m_gui->view->setModel(m_layout->model());
  m_gui->view->setItemDelegate(new SegmentationDelegate(m_baseModel, m_undoStack, m_viewManager)); //TODO 2012-10-05 Sigue sirviendo para algo??

  connect(m_gui->view->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
          this, SLOT(updateSelection(QItemSelection, QItemSelection)));
}

//------------------------------------------------------------------------
void SegmentationExplorer::changeTaxonomy(TaxonomyElement* taxonomy)
{
  SegmentationList selectedSegmentations = m_viewManager->selectedSegmentations();
  foreach(Segmentation *seg, selectedSegmentations)
  {
    m_baseModel->changeTaxonomy(seg, taxonomy);
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
      m_gui->view->selectionModel()->blockSignals(true);
      m_gui->view->selectionModel()->clear();
      m_gui->view->selectionModel()->blockSignals(false);
      m_viewManager->clearSelection(false);
      m_undoStack->beginMacro("Delete Segmentations");
      // BUG: Temporal Fix until RemoveSegmentation's bug is fixed
      foreach(Segmentation *seg, toDelete)
      {
        m_undoStack->push(new RemoveSegmentation(seg, m_baseModel));
      }
      m_undoStack->endMacro();
    }
  }
}


//------------------------------------------------------------------------
void SegmentationExplorer::focusOnSegmentation(const QModelIndex& index)
{
  ModelItem *item = m_layout->item(index);

  if (ModelItem::SEGMENTATION != item->type())
    return;

  Nm bounds[6];
  Segmentation *seg = dynamic_cast<Segmentation*>(item);
  seg->volume()->bounds(bounds);
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
void SegmentationExplorer::rowsAboutToBeRemoved(const QModelIndex parent, int start, int end)
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
void SegmentationExplorer::updateSelection(ViewManager::Selection selection)
{
  if (!isVisible())
    return;

  //qDebug() << "Update Seg Explorer Selection from Selection Manager";
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
  //qDebug() << "Update Selection from Seg Explorer";
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
void SegmentationExplorer::updateSegmentationRepresentations(SegmentationList list)
{
}

//------------------------------------------------------------------------
void SegmentationExplorer::updateSelection()
{
  std::cout << "update selection\n" << std::flush;
}

//------------------------------------------------------------------------
void SegmentationExplorer::changeFinalFlag(bool value)
{
  SegmentationList selectedSegmentations = m_viewManager->selectedSegmentations();
  SegmentationList dependentSegmentations;
  SegmentationList rootSegmentations;

  foreach(Segmentation *seg, selectedSegmentations)
  {
    seg->setFinalNode(value);
    seg->setDependentNode(false);
    if (value)
      seg->setHierarchyRenderingType(HierarchyItem::Opaque, true);
    else
      seg->setHierarchyRenderingType(HierarchyItem::Undefined, false);

    dependentSegmentations.append(seg->components());
    rootSegmentations.append(seg->componentOf());
  }

  foreach(Segmentation *seg, dependentSegmentations)
  {
    if (selectedSegmentations.contains(seg))
    {
      dependentSegmentations.removeAll(seg);
      break;
    }

    selectedSegmentations.append(seg);
    seg->setDependentNode(value);

    if (value)
      seg->setHierarchyRenderingType(HierarchyItem::Hidden, true);
    else
      seg->setHierarchyRenderingType(HierarchyItem::Undefined, false);

    dependentSegmentations.append(seg->components());
  }

  foreach(Segmentation *seg, rootSegmentations)
  {
    if (selectedSegmentations.contains(seg))
    {
      rootSegmentations.removeAll(seg);
      break;
    }

    selectedSegmentations.append(seg);
    seg->setDependentNode(value);

    if (value)
      seg->setHierarchyRenderingType(HierarchyItem::Translucent, true);
    else
      seg->setHierarchyRenderingType(HierarchyItem::Undefined, false);
  }

  foreach(Segmentation *seg, selectedSegmentations)
    seg->notifyModification(true);

  m_viewManager->updateViews();
}