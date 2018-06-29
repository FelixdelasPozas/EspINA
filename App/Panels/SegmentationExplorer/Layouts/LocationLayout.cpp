/*

 Copyright (C) 2018 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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
#include <Core/Utils/ListUtils.hxx>
#include <GUI/Model/Proxies/LocationProxy.h>
#include <App/Panels/SegmentationExplorer/Layouts/LocationLayout.h>
#include <App/Menus/DefaultContextualMenu.h>
#include <GUI/Model/Utils/SegmentationUtils.h>
#include <GUI/Widgets/Styles.h>
#include <Undo/ChangeSegmentationsStack.h>

// Qt
#include <QItemDelegate>
#include <QItemSelection>

using namespace ESPINA;
using namespace ESPINA::Core::Utils;
using namespace ESPINA::GUI::Model::Proxy;
using namespace ESPINA::GUI::Model::Utils;
using namespace ESPINA::GUI::Widgets::Styles;
using namespace ESPINA::Undo;

//--------------------------------------------------------------------
bool LocationLayout::LocationSortFilter::lessThan(const QModelIndex& left, const QModelIndex& right) const
{
  auto leftItem  = itemAdapter(left);
  auto rightItem = itemAdapter(right);

  if(!leftItem)  return false;
  if(!rightItem) return true;

  if (leftItem->type() == rightItem->type())
  {
    return Core::Utils::lessThan<ItemAdapterPtr>(leftItem, rightItem);
  }
  else
  {
    return leftItem->type() == ItemAdapter::Type::CHANNEL;
  }
}

//--------------------------------------------------------------------
LocationLayout::LocationLayout(CheckableTreeView* view, Support::Context& context)
: Layout          {view, context}
, m_sort          {new LocationSortFilter()}
, m_proxy         {new LocationProxy(context.model(), context.viewState())}
, m_orphanSelected{false}
, m_delegate      {new QItemDelegate()}
{
  auto model = context.model();

  m_proxy->setSourceModel(model);
  m_sort->setSourceModel(m_proxy.get());
  m_sort->setDynamicSortFilter(true);

  connect(m_proxy.get(), SIGNAL(segmentationsDropped(SegmentationAdapterList,ChannelAdapterPtr)),
          this,          SLOT(segmentationsDropped(SegmentationAdapterList,ChannelAdapterPtr)));

  connect(model.get(), SIGNAL(rowsInserted(const QModelIndex&, int, int)),
          this,        SLOT(updateSelection()));
  connect(model.get(), SIGNAL(rowsRemoved(const QModelIndex&, int, int)),
          this,        SLOT(updateSelection()));
  connect(model.get(), SIGNAL(modelReset()),
          this,        SLOT(updateSelection()));

  m_sort->sort(m_sort->sortColumn(), m_sort->sortOrder());
}

//--------------------------------------------------------------------
LocationLayout::~LocationLayout()
{
  delete m_delegate;
}

//--------------------------------------------------------------------
void LocationLayout::createSpecificControls(QHBoxLayout* specificControlLayout)
{
}

//--------------------------------------------------------------------
void LocationLayout::contextMenu(const QPoint& pos)
{
  updateSelectedItems();

  auto segmentationsSelected = !m_selectedSegs.isEmpty();
  auto stacksSelected        = !m_selectedStacks.isEmpty();

  QMenu *contextMenu = nullptr;

  if (segmentationsSelected)
  {
    contextMenu = new DefaultContextualMenu(m_selectedSegs, getContext());

    createChangeStackEntry(contextMenu);

    connect(contextMenu, SIGNAL(renamedSegmentations()),
            m_view,      SLOT(update()));
  }

  auto modelStacks = getModel()->channels();

  // returns the index of the given stack pointer in the list of stacks of the model.
  auto stackIndex = [modelStacks](ChannelAdapterPtr stack)
  {
    for(int i = 0; i < modelStacks.size(); ++i)
    {
      if(modelStacks.at(i).get() == stack)
      {
        return i;
        break;
      }
    }

    return -1;
  };

  if (stacksSelected || m_orphanSelected)
  {
    if (!segmentationsSelected)
    {
      contextMenu = new QMenu();
    }
    else
    {
      contextMenu->addSeparator();
    }

    for(auto stack: m_selectedStacks)
    {
      auto selectSameStack = contextMenu->addAction(tr("Select all segmentations from '%1'").arg(stack->data().toString()));
      selectSameStack->setProperty("stackIndex", stackIndex(stack));
      connect(selectSameStack, SIGNAL(triggered(bool)),
              this,            SLOT(selectAllFromStack()));
    }

    if(m_orphanSelected)
    {
      auto selectOrphans = contextMenu->addAction(tr("Select all segmentations without a related stack"));
      connect(selectOrphans, SIGNAL(triggered(bool)),
              this,          SLOT(selectOrphans()));
    }

    if(modelStacks.size() > 1 && m_selectedStacks.size() == 1)
    {
      auto moveToNewStack = contextMenu->addAction(tr("Relocate all segmentations to '%1'").arg(m_selectedStacks.first()->data().toString()));

      connect(moveToNewStack, SIGNAL(triggered(bool)),
              this,           SLOT(moveAllToStack()));
    }
  }

  if (segmentationsSelected && m_selectedSegs.size() == 1)
  {
    contextMenu->addSeparator();

    auto selectSameStack = contextMenu->addAction(tr("Select all segmentations from the same location"));

    auto stack = m_proxy->stackOf(m_selectedSegs.first());
    if(stack)
    {
      selectSameStack->setProperty("stackIndex", stackIndex(stack));
      connect(selectSameStack, SIGNAL(triggered(bool)),
              this,            SLOT(selectAllFromStack()));
    }
    else
    {
      selectSameStack->setText(tr("Select all segmentations without a related stack"));
      connect(selectSameStack, SIGNAL(triggered(bool)),
              this,            SLOT(selectAllOrphans()));
    }
    selectSameStack->setEnabled(!stacksSelected && !m_orphanSelected);
  }

  if(contextMenu)
  {
    contextMenu->exec(pos);

    delete contextMenu;
  }
}

//--------------------------------------------------------------------
void LocationLayout::deleteSelectedItems()
{
  if (!m_selectedSegs.isEmpty())
  {
    auto message  = QObject::tr("Do you really want to delete the selected segmentations?");
    auto title    = QObject::tr("Delete Selected Items");
    auto details  = QObject::tr("Selected to be deleted:");

    for(int i = 0; i < m_selectedSegs.size(); ++i)
    {
      details.append(QString("\n - %1").arg(m_selectedSegs.at(i)->data().toString()));
    }

    if(QMessageBox::Ok == GUI::DefaultDialogs::UserQuestion(message, QMessageBox::Cancel|QMessageBox::Ok, title, details))
    {
      deleteSegmentations(m_selectedSegs);
      m_selectedSegs.clear();
    }
  }
}

//--------------------------------------------------------------------
void LocationLayout::showSelectedItemsInformation()
{
  if(!m_selectedSegs.isEmpty())
  {
    showSegmentationProperties(m_selectedSegs);
  }
}

//--------------------------------------------------------------------
bool LocationLayout::hasInformationToShow()
{
  updateSelectedItems();

  return !m_selectedSegs.isEmpty() && m_selectedStacks.isEmpty();
}

//--------------------------------------------------------------------
QItemDelegate *LocationLayout::itemDelegate() const
{
  return m_delegate;
}

//--------------------------------------------------------------------
void LocationLayout::segmentationsDropped(SegmentationAdapterList segmentations, ChannelAdapterPtr stack)
{
  auto segmentationNames = segmentationListNames(segmentations);
  auto undoStack         = getUndoStack();
  auto number            = segmentations.size() > 1 ? "s":"";

  WaitingCursor cursor;

  undoStack->beginMacro(tr("Relocate segmentation%1 to '%2': %3").arg(number).arg(stack->data().toString()).arg(segmentationNames));
  undoStack->push(new ChangeSegmentationsStack(m_selectedSegs, stack));
  undoStack->endMacro();
}

//--------------------------------------------------------------------
void LocationLayout::updateSelection()
{
  if(isActive())
  {
    m_sort->sort(m_sort->sortColumn(), m_sort->sortOrder());
  }
}

//--------------------------------------------------------------------
void LocationLayout::updateSelectedItems()
{
  m_selectedSegs.clear();
  m_selectedStacks.clear();
  m_orphanSelected = false;
  auto haveOrphans = m_proxy->orphanIndex() != QModelIndex();

  for(auto index : m_view->selectionModel()->selectedIndexes())
  {
    auto selectedItem = item(index);

    if(selectedItem)
    {
      switch(selectedItem->type())
      {
        case ItemAdapter::Type::SEGMENTATION:
          m_selectedSegs << segmentationPtr(selectedItem);
          break;
        case ItemAdapter::Type::CHANNEL:
          m_selectedStacks << channelPtr(selectedItem);
          break;
        default:
          break;
      }
    }
    else
    {
      if(!m_orphanSelected && haveOrphans)
      {
        m_orphanSelected = true;
      }
    }
  }
}

//--------------------------------------------------------------------
void LocationLayout::selectAllFromStack()
{
  auto action = qobject_cast<QAction *>(sender());

  if(action)
  {
    bool ok;
    auto stackIndex = action->property("stackIndex").toInt(&ok);
    auto modelStacks = getModel()->channels();

    if(ok && stackIndex >= 0 && stackIndex < modelStacks.size())
    {
      auto stack = modelStacks.at(stackIndex);

      QItemSelection selection;
      auto stackIndex = index(stack.get());
      auto number = m_proxy->segmentationsOf(stack.get()).size();
      selection.merge(QItemSelection(stackIndex.child(0,0), stackIndex.child(number-1, 0)), QItemSelectionModel::Select);

      if(!selection.isEmpty())
      {
        m_view->selectionModel()->clearSelection();
        m_view->selectionModel()->select(selection, QItemSelectionModel::Select);
        m_view->selectionModel()->setCurrentIndex(selection.first().topLeft(), QItemSelectionModel::Select);
        m_view->scrollTo(selection.first().topLeft(), QAbstractItemView::ScrollHint::EnsureVisible);
      }
    }
  }
}

//--------------------------------------------------------------------
void LocationLayout::selectAllOrphans()
{
  QItemSelection selection;
  auto parentIndex = m_proxy->orphanIndex();
  auto number      = m_proxy->orphanedSegmentations().size();
  selection.merge(QItemSelection(parentIndex.child(0,0), parentIndex.child(number-1, 0)), QItemSelectionModel::Select);

  if(!selection.isEmpty())
  {
    m_view->selectionModel()->clearSelection();
    m_view->selectionModel()->select(selection, QItemSelectionModel::Select);
    m_view->selectionModel()->setCurrentIndex(selection.first().topLeft(), QItemSelectionModel::Select);
    m_view->scrollTo(selection.first().topLeft(), QAbstractItemView::ScrollHint::EnsureVisible);
  }
}

//--------------------------------------------------------------------
void LocationLayout::moveToStack()
{
  auto action = qobject_cast<QAction *>(sender());

  if(action)
  {
    bool ok;
    auto stackIndex  = action->property("stackIndex").toInt(&ok);
    auto modelStacks = getModel()->channels();

    if(ok && stackIndex >= 0 && stackIndex < modelStacks.size())
    {
      auto segmentationNames = segmentationListNames(m_selectedSegs);
      auto stack             = modelStacks.at(stackIndex).get();
      auto undoStack         = getUndoStack();
      auto number            = m_selectedSegs.size() > 1 ? "s":"";

      WaitingCursor cursor;

      undoStack->beginMacro(tr("Relocate segmentation%1 to '%2': %3").arg(number).arg(stack->data().toString()).arg(segmentationNames));
      undoStack->push(new ChangeSegmentationsStack(m_selectedSegs, stack));
      undoStack->endMacro();
    }
  }
  else
  {
    qWarning() << "received signal but couldn't identify sender." << __FILE__ << __LINE__;
  }
}

//--------------------------------------------------------------------
void LocationLayout::moveAllToStack()
{
  auto action = qobject_cast<QAction *>(sender());

  if(action)
  {
    bool ok;
    auto stackIndex  = action->property("stackIndex").toInt(&ok);
    auto modelStacks = getModel()->channels();

    if(ok && stackIndex >= 0 && stackIndex < modelStacks.size())
    {
      SegmentationAdapterList segmentations;
      auto stack = modelStacks.at(stackIndex).get();

      for(auto modelStack: modelStacks)
      {
        if(modelStack.get() == stack) continue;
        segmentations << m_proxy->segmentationsOf(modelStack.get());
      }

      auto segmentationNames = segmentationListNames(segmentations);
      auto undoStack = getUndoStack();
      auto number = segmentations.size() > 1 ? "s":"";

      WaitingCursor cursor;

      undoStack->beginMacro(tr("Relocate segmentation%1 to '%2': %3").arg(number).arg(stack->data().toString()).arg(segmentationNames));
      undoStack->push(new ChangeSegmentationsStack(segmentations, stack));
      undoStack->endMacro();
    }
  }
  else
  {
    qWarning() << "received signal but couldn't identify sender." << __FILE__ << __LINE__;
  }
}

//--------------------------------------------------------------------
void LocationLayout::createChangeStackEntry(QMenu* menu)
{
  auto modelStacks = getModel()->channels();
  auto changeStackMenu = new QMenu(tr("&Change location to"));

  if(modelStacks.size() > 1 || (m_proxy->orphanIndex() != QModelIndex()))
  {
    ChannelAdapterList segmentationStacks;

    for(auto segmentation: m_selectedSegs)
    {
      auto stack = m_proxy->stackOf(segmentation);
      if(stack && !segmentationStacks.contains(stack)) segmentationStacks << stack;
    }

    for(auto stack: modelStacks)
    {
      QAction *action = nullptr;
      if(stack->hue() == -1.)
      {
        action = changeStackMenu->addAction(tr("%1").arg(stack->data().toString()));
      }
      else
      {
        QPixmap icon{32,32};
        icon.fill(QColor::fromHsv(static_cast<int>(stack->hue() * 359), 255,255));

        action = changeStackMenu->addAction(QIcon{icon}, tr("%1").arg(stack->data().toString()));
      }

      auto enabled = (segmentationStacks.size() == 1 && segmentationStacks.first() != stack.get()) || m_orphanSelected;
      if(!enabled)
      {
        action->setEnabled(false);
      }
      else
      {
        action->setProperty("stackIndex", modelStacks.indexOf(stack));

        connect(action, SIGNAL(triggered(bool)),
                this,   SLOT(moveToStack()));
      }
    }
  }
  else
  {
    changeStackMenu->setEnabled(false);
  }

  menu->insertMenu(menu->actions().first(), changeStackMenu);
}

//--------------------------------------------------------------------
ItemAdapterPtr LocationLayout::item(const QModelIndex& index) const
{
  if(!index.isValid() || !isActive()) return nullptr;

  return itemAdapter(m_sort->mapToSource(index));
}

//--------------------------------------------------------------------
QModelIndex LocationLayout::index(ItemAdapterPtr item) const
{
  if(item->type() == ItemAdapter::Type::CHANNEL || item->type() == ItemAdapter::Type::SEGMENTATION)
  {
    auto sourceIndex = Layout::index(item);
    if(sourceIndex.isValid())
    {
      auto proxyIndex = m_proxy->mapFromSource(sourceIndex);
      if(proxyIndex.isValid())
      {
        auto sortIndex = m_sort->mapFromSource(proxyIndex);
        return sortIndex;
      }
    }
  }

  return QModelIndex();
}

