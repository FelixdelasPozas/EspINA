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
#include "Docks/SegmentationExplorer/SegmentationExplorerLayout.h"
#include "LayoutComposition.h"
#include "LayoutLocation.h"
#include "LayoutTaxonomy.h"

// EspINA
#include <Core/Model/EspinaModel.h>
#include <Core/Model/Proxies/TaxonomyProxy.h>
#include <Core/Model/Sample.h>
#include <Core/Model/Segmentation.h>
#include <Core/Model/HierarchyItem.h>
#include <GUI/ISettingsPanel.h>
#include <Undo/RemoveSegmentation.h>

#ifdef TEST_ESPINA_MODELS
#include <Core/Model/ModelTest.h>
#include <Core/Extensions/Tags/TagExtension.h>
#endif

// Qt
#include <QContextMenuEvent>
#include <QMenu>
#include <QCompleter>
#include <QSortFilterProxyModel>
#include <QStringListModel>
#include <QUndoStack>
#include <QWidgetAction>

using namespace EspINA;

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

  showInformationButton->setIcon(
    qApp->style()->standardIcon(QStyle::SP_MessageBoxInformation));
}


//------------------------------------------------------------------------
SegmentationExplorer::SegmentationExplorer(EspinaModel *model,
                                           QUndoStack  *undoStack,
                                           ViewManager *vm,
                                           QWidget     *parent)
: IDockWidget(parent)
, m_baseModel  (model)
, m_undoStack  (undoStack)
, m_viewManager(vm)
, m_gui        (new GUI())
, m_layout     (NULL)
{
  setObjectName("SegmentationExplorer");

  setWindowTitle(tr("Segmentation Explorer"));

  //   addLayout("Debug", new Layout(m_baseModel));
  addLayout("Type",        new TaxonomyLayout   (m_gui->view, m_baseModel, m_undoStack, m_viewManager));
  addLayout("Location",    new LocationLayout   (m_gui->view, m_baseModel, m_undoStack, m_viewManager));
  addLayout("Composition", new CompositionLayout(m_gui->view, m_baseModel, m_undoStack, m_viewManager));

  m_layoutModel.setStringList(m_layoutNames);
  m_gui->groupList->setModel(&m_layoutModel);
  changeLayout(0);

  connect(m_gui->groupList, SIGNAL(currentIndexChanged(int)),
          this, SLOT(changeLayout(int)));
  connect(m_gui->view, SIGNAL(doubleClicked(QModelIndex)),
          this, SLOT(focusOnSegmentation(QModelIndex)));
  connect(m_gui->view, SIGNAL(itemStateChanged(QModelIndex)),
          this, SLOT(updateSegmentationRepresentations()));
  connect(m_gui->showInformationButton, SIGNAL(clicked(bool)),
          this, SLOT(showSelectedItemsInformation()));
  connect(m_gui->deleteButton, SIGNAL(clicked(bool)),
          this, SLOT(deleteSelectedItems()));
  connect(m_viewManager, SIGNAL(selectionChanged(ViewManager::Selection, bool)),
          this, SLOT(updateSelection(ViewManager::Selection)));
  connect(m_gui->searchText, SIGNAL(textChanged(QString)),
          this, SLOT(updateSearchFilter()));

  setWidget(m_gui);

  m_gui->view->installEventFilter(this);
}

//------------------------------------------------------------------------
SegmentationExplorer::~SegmentationExplorer()
{
  qDebug() << "********************************************************";
  qDebug() << "          Destroying Segmentation Explorer";
  qDebug() << "********************************************************";
  foreach(Layout *layout, m_layouts)
    delete layout;
}

//------------------------------------------------------------------------
void SegmentationExplorer::initDockWidget(EspinaModel *model,
                                          QUndoStack  *undoStack,
                                          ViewManager *viewManager)
{
}

//------------------------------------------------------------------------
void SegmentationExplorer::reset()
{
  foreach(Layout *layout, m_layouts)
    layout->reset();
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
  if (sender == m_gui->view && QEvent::ContextMenu == e->type() && m_layout)
  {
    QContextMenuEvent *cme = static_cast<QContextMenuEvent *>(e);

    m_layout->contextMenu(cme->globalPos());

    return true;
  }

  return QObject::eventFilter(sender, e);
}

//------------------------------------------------------------------------
void SegmentationExplorer::updateGUI(const QModelIndexList &selectedIndexes)
{
  m_gui->showInformationButton->setEnabled(!selectedIndexes.empty());
  m_gui->deleteButton->setEnabled(!selectedIndexes.empty());

  QSet<QString> tagSet;
  foreach(QModelIndex index, selectedIndexes)
  {
    ModelItemPtr item = m_layout->item(index);
    if (EspINA::SEGMENTATION == item->type())
    {
      SegmentationPtr segmentation = segmentationPtr(item);
      tagSet.unite(segmentation->information(SegmentationTags::TAGS).toStringList().toSet());
    }
  }
  QStringList tags = tagSet.toList();
  m_gui->selectedTags->setText(tags.join(","));
}

//------------------------------------------------------------------------
void SegmentationExplorer::changeLayout(int index)
{
  Q_ASSERT(index < m_layouts.size());
  if (m_layout)
  {
    disconnect(m_gui->view->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
               this, SLOT(updateSelection(QItemSelection, QItemSelection)));

    QLayoutItem *specificControl;
    while ((specificControl = m_gui->specificControlLayout->takeAt(0)) != 0)
    {
      delete specificControl->widget();
      delete specificControl;
    }
  }

  m_layout = m_layouts[index];
#ifdef TEST_ESPINA_MODELS
  m_modelTester = QSharedPointer<ModelTest>(new ModelTest(m_layout->model()));
#endif
  m_gui->view->setModel(m_layout->model());
  QCompleter *completer = new QCompleter(&SegmentationTags::TagModel, this);
  completer->setCaseSensitivity(Qt::CaseInsensitive);
  completer->setCompletionMode(QCompleter::UnfilteredPopupCompletion);
  completer->setModelSorting(QCompleter::CaseInsensitivelySortedModel);
  m_gui->searchText->setCompleter(completer);

  connect(m_gui->view->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
          this, SLOT(updateSelection(QItemSelection, QItemSelection)));

  m_layout->createSpecificControls(m_gui->specificControlLayout);

  m_gui->view->setItemDelegate(m_layout->itemDelegate());
}

//------------------------------------------------------------------------
void SegmentationExplorer::deleteSelectedItems()
{
  if (m_layout)
  {
    m_layout->deleteSelectedItems();
  }
}

//------------------------------------------------------------------------
void SegmentationExplorer::showSelectedItemsInformation()
{
  if (m_layout)
    m_layout->showSelectedItemsInformation();

  return;
}

//------------------------------------------------------------------------
void SegmentationExplorer::focusOnSegmentation(const QModelIndex& index)
{
  ModelItemPtr item = m_layout->item(index);

  if (EspINA::SEGMENTATION != item->type())
    return;

  Nm bounds[6];
  SegmentationPtr seg = segmentationPtr(item);
  seg->volume()->bounds(bounds);
  Nm center[3] = { (bounds[0] + bounds[1])/2, (bounds[2] + bounds[3])/2, (bounds[4] + bounds[5])/2 };
  m_viewManager->focusViewsOn(center);
}

//------------------------------------------------------------------------
void SegmentationExplorer::updateSelection(ViewManager::Selection selection)
{
  if (!isVisible() || signalsBlocked())
    return;

  m_gui->view->blockSignals(true);
  m_gui->view->selectionModel()->blockSignals(true);
  m_gui->view->selectionModel()->reset();
  foreach(PickableItemPtr item, selection)
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

  updateGUI(m_gui->view->selectionModel()->selection().indexes());

  // Update all visible items
  m_gui->view->viewport()->update();
}

//------------------------------------------------------------------------
void SegmentationExplorer::updateSelection(QItemSelection selected, QItemSelection deselected)
{
  ViewManager::Selection selection;
  QModelIndexList selectedIndexes = m_gui->view->selectionModel()->selection().indexes();
  foreach(QModelIndex index, selectedIndexes)
  {
    ModelItemPtr item = m_layout->item(index);
    if (EspINA::SEGMENTATION == item->type())
      selection << pickableItemPtr(item);
  }

  updateGUI(selectedIndexes);
//   m_gui->showInformationButton->setEnabled(!selection.empty());
//   m_gui->deleteButton->setEnabled(!selectedIndexes.empty());

  // signal blocking is necessary because we don't want to change our current selection indexes,
  // and that will happen if a updateSelection(ViewManager::Selection) is called.
  this->blockSignals(true);
  m_viewManager->setSelection(selection);
  this->blockSignals(false);
}

//------------------------------------------------------------------------
void SegmentationExplorer::updateSegmentationRepresentations(SegmentationList list)
{
  m_viewManager->updateSegmentationRepresentations(list);
  m_viewManager->updateViews();
}

//------------------------------------------------------------------------
void SegmentationExplorer::updateChannelRepresentations(ChannelList list)
{
  m_viewManager->updateChannelRepresentations(list);
  m_viewManager->updateViews();
}

//------------------------------------------------------------------------
void SegmentationExplorer::updateSelection()
{
}

//------------------------------------------------------------------------
void SegmentationExplorer::updateSearchFilter()
{
  m_gui->clearSearch->setEnabled(!m_gui->searchText->text().isEmpty());

  m_layout->setFilterRegExp(m_gui->searchText->text());
}
