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
#include "DefaultContextualMenu.h"
#include <Extensions/ExtensionUtils.h>
#include <Extensions/Notes/SegmentationNotes.h>
#include <Extensions/Tags/SegmentationTags.h>
#include <GUI/Widgets/NoteEditor.h>
#include <GUI/Widgets/TagSelector.h>
#include <Undo/ChangeSegmentationNotes.h>
#include <Undo/ChangeSegmentationTags.h>
#include <Undo/ChangeCategoryCommand.h>
#include <Undo/RenameSegmentationsCommand.h>
#include <Undo/RemoveSegmentations.h>

// Qt
#include <QWidgetAction>
#include <QTreeView>
#include <QHeaderView>
#include <QStringList>
#include <QStringListModel>
#include <QListView>
#include <QInputDialog>
#include <QStandardItemModel>
#include <QMessageBox>

using namespace ESPINA;

//------------------------------------------------------------------------
DefaultContextualMenu::DefaultContextualMenu(SegmentationAdapterList selection,
                                             Support::Context &context,
                                             QWidget                *parent)
: ContextualMenu  {parent}
, m_context       (context)
, m_classification{nullptr}
, m_segmentations {selection}
{
  createChangeCategoryMenu();
  createNoteEntry();
  createTagsEntry();
  createRenameEntry();
  createDeleteEntry();
}

//------------------------------------------------------------------------
DefaultContextualMenu::~DefaultContextualMenu()
{
  if (m_classification)
  {
    delete m_classification;
  }
}

//------------------------------------------------------------------------
void DefaultContextualMenu::addNote()
{
  QList<QUndoCommand *> commands;

  for(auto segmentation : m_segmentations)
  {
    QString previousNotes;

    if (segmentation->hasExtension(SegmentationNotes::TYPE))
    {
      previousNotes = segmentation->information(SegmentationNotes::NOTES).toString();
    }

    NoteEditor editor(segmentation->data().toString(), previousNotes);

    if (editor.exec())
    {
      commands << new ChangeSegmentationNotes(segmentation, editor.text());
    }
  }

  if (!commands.isEmpty())
  {
    auto undoStack = m_context.undoStack();

    undoStack->beginMacro(tr("Add Notes"));
    for (auto command : commands)
    {
      undoStack->push(command);
    }
    undoStack->endMacro();
  }
}

//------------------------------------------------------------------------
void DefaultContextualMenu::changeSegmentationsCategory(const QModelIndex& index)
{
   this->hide();

   ItemAdapterPtr categoryItem = itemAdapter(index);
   Q_ASSERT(isCategory(categoryItem));
   CategoryAdapterPtr categoryAdapter = toCategoryAdapterPtr(categoryItem);

   auto undoStack = m_context.undoStack();
   undoStack->beginMacro(tr("Change Category"));
   {
     undoStack->push(new ChangeCategoryCommand(m_segmentations, categoryAdapter, m_context));
   }
   undoStack->endMacro();

   emit changeCategory(categoryAdapter);
}

//------------------------------------------------------------------------
void DefaultContextualMenu::manageTags()
{
  QList<QUndoCommand *> commands;

  if (!m_segmentations.isEmpty())
  {
    QStandardItemModel tags;

    TagSelector tagSelector(dialogTitle(), tags);
    if (tagSelector.exec())
    {
      for(auto segmentation : m_segmentations)
      {
        QStringList currentTags, previousTags;

        if (segmentation->hasExtension(SegmentationTags::TYPE))
        {
          currentTags  = segmentation->information(SegmentationTags::TAGS).toString().split(";");
          previousTags = currentTags;
        }

        for (int r = 0; r < tags.rowCount(); ++r)
        {
          QStandardItem *item = tags.item(r);
          if (Qt::Checked == item->checkState())
          {
            if (!currentTags.contains(item->text()))
              currentTags << item->text();
          } else if (Qt::Unchecked == item->checkState())
          {
            currentTags.removeAll(item->text());
          }
        }
        if (previousTags != currentTags)
          commands << new ChangeSegmentationTags(segmentation, currentTags);
      }
    }

  }

  if (!commands.isEmpty())
  {
    auto undoStack = m_context.undoStack();

    undoStack->beginMacro(tr("Change Segmentation Tags"));
    for (auto command : commands)
    {
      undoStack->push(command);
    }
    undoStack->endMacro();
  }
}

//------------------------------------------------------------------------
void DefaultContextualMenu::resetRootItem()
{
  m_classification->setRootIndex(m_context.model()->classificationRoot());
}

//------------------------------------------------------------------------
void DefaultContextualMenu::renameSegmentation()
{
  QMap<SegmentationAdapterPtr, QString> renames;

  for (auto segmentation : m_segmentations)
  {
    QString oldName = segmentation->data().toString();
    QString alias = QInputDialog::getText(this, oldName, "Rename Segmentation", QLineEdit::Normal, oldName);

    bool exists = false;
    for (auto existinSegmentation : m_context.model()->segmentations())
    {
      exists |= (existinSegmentation->data().toString() == alias && segmentation != existinSegmentation.get());
    }

    if (exists)
    {
      QMessageBox::warning(this, tr("Alias duplicated"),
          tr("Segmentation alias is already used by another segmentation."));
    }
    else
    {
      renames[segmentation] = alias;
    }
  }

  if (renames.size() != 0)
  {
    auto undoStack = m_context.undoStack();
    undoStack->beginMacro(QString("Rename segmentations"));
    undoStack->push(new RenameSegmentationsCommand(renames));
    undoStack->endMacro();
  }
}

//------------------------------------------------------------------------
void DefaultContextualMenu::deleteSelectedSementations()
{
  this->hide();

  auto undoStack = m_context.undoStack();
  undoStack->beginMacro("Delete Segmentations");
  undoStack->push(new RemoveSegmentations(m_segmentations, m_context.model()));
  undoStack->endMacro();

  emit deleteSegmentations();
}

//------------------------------------------------------------------------
void DefaultContextualMenu::setSelection(SelectionSPtr selection)
{
}

//------------------------------------------------------------------------
void DefaultContextualMenu::createNoteEntry()
{
  QAction *noteAction = addAction(tr("Notes"));
  noteAction->setIcon(QIcon(":/espina/note.png"));
  connect(noteAction, SIGNAL(triggered(bool)),
          this, SLOT(addNote()));
}

//------------------------------------------------------------------------
void DefaultContextualMenu::createChangeCategoryMenu()
{
   QMenu         *changeCategoryMenu = new QMenu(tr("Change Category"));
   QWidgetAction *categoryListAction = new QWidgetAction(changeCategoryMenu);

   auto model = m_context.model();

   m_classification = new QTreeView();
   m_classification->header()->setVisible(false);
   m_classification->setModel(model.get());
   m_classification->setRootIndex(model->classificationRoot());
   m_classification->expandAll();
   connect(model.get(), SIGNAL(modelReset()),
           this,          SLOT(resetRootItem()));
   connect(m_classification, SIGNAL(clicked(QModelIndex)),
           this, SLOT(changeSegmentationsCategory(QModelIndex)));

   categoryListAction->setDefaultWidget(m_classification);
   changeCategoryMenu->addAction(categoryListAction);
   addMenu(changeCategoryMenu);
}

//------------------------------------------------------------------------
void DefaultContextualMenu::createTagsEntry()
{
  QAction *tagsAction = addAction(tr("Tags"));
  tagsAction->setIcon(QIcon(":/espina/tag.svg"));

  connect(tagsAction, SIGNAL(triggered(bool)),
          this,       SLOT(manageTags()));

}

//------------------------------------------------------------------------
void DefaultContextualMenu::createRenameEntry()
{
  QAction *action = addAction(tr("&Rename"));
  connect(action, SIGNAL(triggered(bool)),
          this, SLOT(renameSegmentation()));
}

//------------------------------------------------------------------------
QString DefaultContextualMenu::dialogTitle() const
{
  QString title = m_segmentations[0]->data().toString();

  if (m_segmentations.size() > 1)
  {
    title.append(", " + m_segmentations[1]->data().toString());
  }

  if (m_segmentations.size() > 2)
  {
    title.append(", ...");
  }

  return title;
}

//------------------------------------------------------------------------
void DefaultContextualMenu::createDeleteEntry()
{
  QAction *deleteAction = addAction(tr("Delete"));

  connect(deleteAction, SIGNAL(triggered(bool)),
          this,         SLOT(deleteSelectedSementations()));
}
