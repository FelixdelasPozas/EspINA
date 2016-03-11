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
#include "Panels/SegmentationExplorer/SegmentationExplorerLayout.h"
#include "Layouts/ClassificationLayout.h"
#include <Extensions/Tags/SegmentationTags.h>
#include <Extensions/ExtensionUtils.h>
#include <GUI/Model/Utils/SegmentationUtils.h>
#include <GUI/Utils/Format.h>

// Qt
#include <QContextMenuEvent>
#include <QMenu>
#include <QCompleter>
#include <QShortcut>
#include <QSortFilterProxyModel>
#include <QStringListModel>
#include <QUndoStack>
#include <QWidgetAction>

using namespace ESPINA;
using namespace ESPINA::GUI::Utils::Format;
using namespace ESPINA::GUI::Model::Utils;

//------------------------------------------------------------------------
class SegmentationExplorer::GUI
: public QWidget
, public Ui::SegmentationExplorer
{
public:
  /** \brief UI class constructor.
   *
   */
  GUI()
  {
    setupUi(this);
    groupLabel->setVisible(false);
    groupList->setVisible(false);
    view->setSortingEnabled(true);
    view->sortByColumn(0, Qt::AscendingOrder);
  }
};

//------------------------------------------------------------------------
SegmentationExplorer::SegmentationExplorer(Support::FilterRefinerFactory &filterRefiners,
                                           Support::Context &context)
: Panel(tr("Segmentation Explorer"), context)
, SelectableView(context.viewState())
, m_gui   {new GUI()}
, m_layout{nullptr}
{
  setObjectName("SegmentationExplorer");

  //   addLayout("Debug", new Layout(m_baseModel));
  addLayout(tr("Category"), new ClassificationLayout(m_gui->view, filterRefiners, context));

  m_layoutModel.setStringList(m_layoutNames);
  m_gui->groupList->setModel(&m_layoutModel);
  changeLayout(0);

  connect(m_gui->groupList, SIGNAL(currentIndexChanged(int)),
          this,             SLOT(changeLayout(int)));
  connect(m_gui->view, SIGNAL(doubleClicked(QModelIndex)),
          this,        SLOT(focusOnSegmentation(QModelIndex)));
  connect(m_gui->showInformationButton, SIGNAL(clicked(bool)),
          this,                         SLOT(showSelectedItemsInformation()));
  connect(m_gui->deleteButton, SIGNAL(clicked(bool)),
          this,                SLOT(deleteSelectedItems()));
  connect(m_gui->searchText, SIGNAL(textChanged(QString)),
          this,              SLOT(updateSearchFilter()));
  connect(m_gui->selectedTags, SIGNAL(linkActivated(QString)),
          this,                SLOT(onTagSelected(QString)));

  connect(currentSelection().get(), SIGNAL(selectionStateChanged()),
          this,                     SLOT(onSelectionChanged()));

  setWidget(m_gui);

  m_gui->view->installEventFilter(this);
  m_gui->selectedTags->setOpenExternalLinks(false);

  createShortcuts();
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
void SegmentationExplorer::addLayout(const QString &id, SegmentationExplorer::Layout* proxy)
{
  m_layoutNames << id;
  m_layouts << proxy;
}

//------------------------------------------------------------------------
bool SegmentationExplorer::eventFilter(QObject *sender, QEvent *e)
{
  if (sender == m_gui->view && QEvent::ContextMenu == e->type() && m_layout)
  {
    auto cme = static_cast<QContextMenuEvent *>(e);

    m_layout->contextMenu(cme->globalPos());

    updateTags(selectedIndexes());

    return true;
  }

  return QObject::eventFilter(sender, e);
}

//------------------------------------------------------------------------
void SegmentationExplorer::updateGUI(const QModelIndexList &selectedIndexes)
{
  m_gui->showInformationButton->setEnabled(m_layout->hasInformationToShow());
  m_gui->deleteButton->setEnabled(!selectedIndexes.empty());

  updateTags(selectedIndexes);
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

  if(m_layout)
  {
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
  }

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
}

//------------------------------------------------------------------------
void SegmentationExplorer::focusOnSegmentation(const QModelIndex& index)
{
  ItemAdapterPtr item;

  if(m_layout)
  {
    item = m_layout->item(index);
  }

  if (item && isSegmentation(item))
  {
    auto segmentation       = segmentationPtr(item);
    auto segmentationCenter = centroid(segmentation->bounds());

    getViewState().focusViewOn(segmentationCenter);
  }
}

//------------------------------------------------------------------------
void SegmentationExplorer::onModelSelectionChanged(QItemSelection selected, QItemSelection deselected)
{
  ViewItemAdapterList selection;

  auto indexes = selectedIndexes();

  for(auto index: indexes)
  {
    auto item = m_layout->item(index);
    if (isSegmentation(item))
    {
      selection << viewItemAdapter(item);
    }
  }

  updateGUI(indexes);

  // signal blocking is necessary because we don't want to change our current selection indexes,
  // and that will happen if a updateSelection(ViewManager::Selection) is called.
  this->blockSignals(true);
  currentSelection()->set(selection);
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
  {
    return;
  }

  m_gui->view->blockSignals(true);
  m_gui->view->selectionModel()->blockSignals(true);
  m_gui->view->selectionModel()->reset();

  auto selection =  currentSelection()->items();
  for(auto item : selection)
  {
    auto index = m_layout->index(item);

    if (index.isValid())
    {
      m_gui->view->selectionModel()->select(index, QItemSelectionModel::Select);
    }
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
void SegmentationExplorer::onTagSelected(const QString& tag)
{
  m_gui->searchText->setText(tag);
}

//------------------------------------------------------------------------
QModelIndexList SegmentationExplorer::selectedIndexes() const
{
  return m_gui->view->selectionModel()->selectedIndexes();
}

//------------------------------------------------------------------------
void SegmentationExplorer::updateTags(const QModelIndexList &selectedIndexes)
{
  QSet<QString> tagSet;
  for(QModelIndex index : selectedIndexes)
  {
    auto item = m_layout->item(index);
    if (isSegmentation(item))
    {
      auto segmentation = segmentationPtr(item);
      auto extensions   = segmentation->extensions();

      if (extensions->hasExtension(SegmentationTags::TYPE))
      {
        auto extension = retrieveExtension<SegmentationTags>(extensions);
        tagSet.unite(extension->tags().toSet());
      }
    }
  }

  QString tagLinks;

  if (!tagSet.isEmpty())
  {
    QStringList tags = tagSet.toList();
    tags.sort();

    for (auto tag : tags)
    {
      tagLinks += createLink(tag);
    }

    m_gui->selectedTags->setText(tagLinks);
  }
  else
  {
    m_gui->selectedTags->clear();
  }
}

//------------------------------------------------------------------------
void SegmentationExplorer::createShortcuts()
{
  QKeySequence incrementSequence, decrementSequence;
  incrementSequence = Qt::Key_PageDown;
  decrementSequence = Qt::Key_PageUp;

  auto increment = new QShortcut(incrementSequence, this, 0, 0, Qt::ApplicationShortcut);
  connect(increment, SIGNAL(activated()),
          this, SLOT(incrementSelection()));

  auto decrement = new QShortcut(decrementSequence, this, 0, 0, Qt::ApplicationShortcut);
  connect(decrement, SIGNAL(activated()),
          this, SLOT(decrementSelection()));
}

//------------------------------------------------------------------------
QModelIndex SegmentationExplorer::nextIndex(const QModelIndex &index, direction dir)
{
  QModelIndex result;
  bool found = false;
  auto begin = index;

  while(!found)
  {
    if(dir == direction::FORWARD)
    {
      result = m_gui->view->indexBelow(begin);
    }
    else
    {
      result = m_gui->view->indexAbove(begin);
    }

    if(!result.isValid() || (result == begin))
    {
      result = QModelIndex();
      found = true;
    }
    else
    {
      if(result.model()->hasChildren(result))
      {
        m_gui->view->expand(result);
        begin = result;
      }
      else
      {
        if(isSegmentation(m_layout->item(result)))
        {
          found = true;
        }
        else
        {
          begin = result;
        }
      }
    }
  }

  return result;
}

//------------------------------------------------------------------------
void SegmentationExplorer::incrementSelection()
{
  auto selection = selectedIndexes();

  if(selection.size() == 1)
  {
    auto index = selection.first();
    auto next  = nextIndex(index, direction::FORWARD);

    if(next != QModelIndex())
    {
      m_gui->view->selectionModel()->select(next, QItemSelectionModel::ClearAndSelect);
      focusOnSegmentation(next);
      return;
    }
  }
}

//------------------------------------------------------------------------
void SegmentationExplorer::decrementSelection()
{
  auto selection = selectedIndexes();

  if(selection.size() == 1)
  {
    auto index = selection.first();
    auto next  = nextIndex(index, direction::BACKWARD);

    if(next != QModelIndex())
    {
      m_gui->view->selectionModel()->select(next, QItemSelectionModel::ClearAndSelect);
      focusOnSegmentation(next);
      return;
    }
  }
}
