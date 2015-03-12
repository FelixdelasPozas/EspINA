/*

    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>

    This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
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

// ESPINA
#include "SegmentationExplorer.h"
#include "Dialogs/SegmentationInspector/SegmentationInspector.h"
#include "Docks/SegmentationExplorer/SegmentationExplorerLayout.h"
#include "Layouts/ClassificationLayout.h"
#include <Extensions/Tags/SegmentationTags.h>
#include <Extensions/ExtensionUtils.h>

// Qt
#include <QContextMenuEvent>
#include <QMenu>
#include <QCompleter>
#include <QSortFilterProxyModel>
#include <QStringListModel>
#include <QUndoStack>
#include <QWidgetAction>

using namespace ESPINA;

//------------------------------------------------------------------------
class SegmentationExplorer::GUI
: public QWidget
, public Ui::SegmentationExplorer
{
public:
	/** \brief GUI class constructor.
	 *
	 */
  GUI()
  {
		setupUi(this);
		view->setSortingEnabled(true);
		view->sortByColumn(0, Qt::AscendingOrder);

		showInformationButton->setIcon(qApp->style()->standardIcon(QStyle::SP_MessageBoxInformation));
	}
};

//------------------------------------------------------------------------
SegmentationExplorer::SegmentationExplorer(ModelAdapterSPtr          model,
                                           ModelFactorySPtr          factory,
                                           FilterDelegateFactorySPtr delegateFactory,
                                           ViewManagerSPtr           viewManager,
                                           QUndoStack               *undoStack,
                                           QWidget                  *parent)
: DockWidget   {parent}
, m_baseModel  {model}
, m_viewManager{viewManager}
, m_undoStack  {undoStack}
, m_gui        {new GUI()}
, m_layout     {nullptr}
{
  setObjectName("SegmentationExplorer");

  setWindowTitle(tr("Segmentation Explorer"));

  //   addLayout("Debug", new Layout(m_baseModel));
  addLayout("Category",    new ClassificationLayout(m_gui->view, m_baseModel, factory, delegateFactory, m_viewManager, m_undoStack));

  m_layoutModel.setStringList(m_layoutNames);
  m_gui->groupList->setModel(&m_layoutModel);
  changeLayout(0);

  connect(m_gui->groupList, SIGNAL(currentIndexChanged(int)),
          this, SLOT(changeLayout(int)));
  connect(m_gui->view, SIGNAL(doubleClicked(QModelIndex)),
          this, SLOT(focusOnSegmentation(QModelIndex)));
  connect(m_gui->view, SIGNAL(itemStateChanged(QModelIndex)),
          this, SLOT(onItemModified()));
  connect(m_gui->showInformationButton, SIGNAL(clicked(bool)),
          this, SLOT(showSelectedItemsInformation()));
  connect(m_gui->deleteButton, SIGNAL(clicked(bool)),
          this, SLOT(deleteSelectedItems()));
  connect(m_gui->searchText, SIGNAL(textChanged(QString)),
          this, SLOT(updateSearchFilter()));

  setWidget(m_gui);

  m_gui->view->installEventFilter(this);
  m_gui->tagsLabel->setVisible(false);
  m_gui->selectedTags->setVisible(false);
}

//------------------------------------------------------------------------
SegmentationExplorer::~SegmentationExplorer()
{
//   qDebug() << "********************************************************";
//   qDebug() << "          Destroying Segmentation Explorer";
//   qDebug() << "********************************************************";
  for(auto layout : m_layouts)
  {
  	delete layout;
  }
}

//------------------------------------------------------------------------
void SegmentationExplorer::reset()
{
  for(auto layout : m_layouts)
  {
  	layout->reset();
  }
}

//------------------------------------------------------------------------
void SegmentationExplorer::onSelectionSet(SelectionSPtr selection)
{
  connect(selection.get(), SIGNAL(selectionStateChanged()),
          this, SLOT(onSelectionChanged()));
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
  m_gui->showInformationButton->setEnabled(m_layout->hasInformationToShow());
  m_gui->deleteButton->setEnabled(!selectedIndexes.empty());

  QSet<QString> tagSet;
  for(QModelIndex index : selectedIndexes)
  {
    auto item = m_layout->item(index);
    if (isSegmentation(item))
    {
      auto segmentation = segmentationPtr(item);
      if (segmentation->hasExtension(SegmentationTags::TYPE))
      {
        auto extension = retrieveExtension<SegmentationTags>(segmentation);
        tagSet.unite(extension->tags().toSet());
      }
    }
  }

  QStringList tags = tagSet.toList();
  tags.sort();
  m_gui->selectedTags->setText(tags.join(", "));

  bool tagVisibility = !tags.isEmpty();
  m_gui->tagsLabel->setVisible(tagVisibility);
  m_gui->selectedTags->setVisible(tagVisibility);
}

//------------------------------------------------------------------------
void SegmentationExplorer::changeLayout(int index)
{
  Q_ASSERT(index < m_layouts.size());

  if (m_layout)
  {
    disconnect(m_gui->view->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
               this,                          SLOT(onModelSelectionChanged(QItemSelection, QItemSelection)));

    disconnect(m_layout->model(), SIGNAL(rowsInserted(const QModelIndex&, int, int)),
               this,              SLOT(updateSelection()));
    disconnect(m_layout->model(), SIGNAL(rowsRemoved(const QModelIndex&, int, int)),
               this,              SLOT(updateSelection()));
    disconnect(m_layout->model(), SIGNAL(modelReset()),
               this,              SLOT(updateSelection()));

    QLayoutItem *specificControl;
    while ((specificControl = m_gui->specificControlLayout->takeAt(0)) != 0)
    {
      delete specificControl->widget();
      delete specificControl;
    }
  }

  m_layout = m_layouts[index];

  m_gui->view->setModel(m_layout->model());

  connect(m_gui->view->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
          this,                          SLOT(onModelSelectionChanged(QItemSelection,QItemSelection)));

  connect(m_layout->model(), SIGNAL(rowsInserted(const QModelIndex&, int, int)),
          this,              SLOT(onSelectionChanged()));
  connect(m_layout->model(), SIGNAL(rowsRemoved(const QModelIndex&, int, int)),
          this,              SLOT(onSelectionChanged()));
  connect(m_layout->model(), SIGNAL(modelReset()),
          this,              SLOT(onSelectionChanged()));

  m_layout->createSpecificControls(m_gui->specificControlLayout);

  m_gui->view->setItemDelegate(m_layout->itemDelegate());
  m_gui->showInformationButton->setEnabled(false);

  onSelectionChanged();
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
  {
    m_layout->showSelectedItemsInformation();
  }

  return;
}

//------------------------------------------------------------------------
void SegmentationExplorer::focusOnSegmentation(const QModelIndex& index)
{
  auto item = m_layout->item(index);

  if (ItemAdapter::Type::SEGMENTATION != item->type())
    return;

  auto segmentation = segmentationPtr(item);
  Bounds bounds = segmentation->output()->bounds();
  NmVector3 center{(bounds[0] + bounds[1])/2, (bounds[2] + bounds[3])/2, (bounds[4] + bounds[5])/2};
  m_viewManager->focusViewsOn(center);
}

//------------------------------------------------------------------------
void SegmentationExplorer::onModelSelectionChanged(QItemSelection selected, QItemSelection deselected)
{
  ViewItemAdapterList selection;

  QModelIndexList selectedIndexes = m_gui->view->selectionModel()->selection().indexes();
  for(auto index: selectedIndexes)
  {
    auto item = m_layout->item(index);
    if (ItemAdapter::Type::SEGMENTATION == item->type())
      selection << viewItemAdapter(item);
  }

  updateGUI(selectedIndexes);

  // signal blocking is necessary because we don't want to change our current selection indices,
  // and that will happen if a updateSelection(ViewManager::Selection) is called.
  this->blockSignals(true);
  m_viewManager->selection()->set(selection);
  this->blockSignals(false);
}

//------------------------------------------------------------------------
void SegmentationExplorer::updateSearchFilter()
{
  m_gui->clearSearch->setEnabled(!m_gui->searchText->text().isEmpty());

  m_layout->setFilterRegExp(m_gui->searchText->text());
}

//------------------------------------------------------------------------
void SegmentationExplorer::onSelectionChanged()
{
  if (!isVisible() || signalsBlocked())
    return;

  m_gui->view->blockSignals(true);
  m_gui->view->selectionModel()->blockSignals(true);
  m_gui->view->selectionModel()->reset();

  auto selection =  currentSelection()->items();
  for(auto item : selection)
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
void SegmentationExplorer::onItemModified()
{
  // TODO: invalidate representations?
}
