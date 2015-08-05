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
#include <GUI/Widgets/NoteEditor.h>
#include <Undo/ChangeSegmentationNotes.h>
#include <Undo/ChangeCategoryCommand.h>
#include <Undo/RenameSegmentationsCommand.h>
#include <Undo/RemoveSegmentations.h>
#include <Support/Utils/TagUtils.h>

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
using namespace ESPINA::GUI;

//------------------------------------------------------------------------
DefaultContextualMenu::DefaultContextualMenu(SegmentationAdapterList selection,
                                             Support::Context       &context,
                                             QWidget                *parent)
: ContextualMenu  (parent)
, WithContext     (context)
, m_classification{nullptr}
, m_segmentations {selection}
{
  createChangeCategoryMenu();
  createNoteEntry();
  createTagsEntry();
  createRenameEntry();
  createExportEntry();
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

    auto extensions = segmentation->readOnlyExtensions();
    if (extensions->hasExtension(SegmentationNotes::TYPE))
    {
      previousNotes = extensions->get<SegmentationNotes>()->notes();
    }

    NoteEditor editor(segmentation->data().toString(), previousNotes);

    if (editor.exec())
    {
      commands << new ChangeSegmentationNotes(segmentation, editor.text());
    }
  }

  if (!commands.isEmpty())
  {
    auto undoStack = getUndoStack();

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

   auto undoStack = getUndoStack();
   undoStack->beginMacro(tr("Change Category"));
   {
     undoStack->push(new ChangeCategoryCommand(m_segmentations, categoryAdapter, getContext()));
   }
   undoStack->endMacro();

   emit changeCategory(categoryAdapter);
}

//------------------------------------------------------------------------
void DefaultContextualMenu::manageTags()
{
  Support::Utils::Tags::manageTags(m_segmentations, getUndoStack());
}

//------------------------------------------------------------------------
void DefaultContextualMenu::resetRootItem()
{
  m_classification->setRootIndex(getModel()->classificationRoot());
}

//------------------------------------------------------------------------
void DefaultContextualMenu::renameSegmentation()
{
  auto renameTitle = tr("Rename Segmentation");

  QMap<SegmentationAdapterPtr, QString> renames;

  for (auto segmentation : m_segmentations)
  {
    QString oldName = segmentation->data().toString();
    QString alias = QInputDialog::getText(this, oldName, renameTitle, QLineEdit::Normal, oldName);

    bool exists = false;
    for (auto existinSegmentation : getModel()->segmentations())
    {
      exists |= (existinSegmentation->data().toString() == alias && segmentation != existinSegmentation.get());
    }

    if (exists)
    {
      auto title = tr("Alias duplicated");
      auto msg   = tr("Segmentation alias is already used by another segmentation.");

      DefaultDialogs::InformationMessage(title, msg);
    }
    else
    {
      renames[segmentation] = alias;
    }
  }

  if (renames.size() != 0)
  {
    auto undoStack = getUndoStack();
    undoStack->beginMacro(renameTitle);
    undoStack->push(new RenameSegmentationsCommand(renames));
    undoStack->endMacro();
  }
}

//------------------------------------------------------------------------
void DefaultContextualMenu::exportSelectedSegmentations()
{
  auto title     = tr("Export selected segmentations");
  auto overwrite = tr("%1 already exists. Do you want to overwrite it?");

  auto dir = DefaultDialogs::SaveDirectory(title);

  for (auto segmentation : m_segmentations)
  {
    auto alias  = segmentation->data(Qt::DisplayRole);
    auto file   = QString("%1.tif").arg(alias.toString());

    if (!dir.exists(file) || DefaultDialogs::UserConfirmation(title, overwrite.arg(file)))
    {
      auto path   = dir.absoluteFilePath(file);
      auto volume = readLockVolume(segmentation->output());

      exportVolume<itkVolumeType>(volume->itkImage(), path);
    }
  }
}

//------------------------------------------------------------------------
void DefaultContextualMenu::deleteSelectedSementations()
{
  this->hide();

  auto undoStack = getUndoStack();
  undoStack->beginMacro(tr("Delete Segmentations"));
  undoStack->push(new RemoveSegmentations(m_segmentations, getModel()));
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
  noteAction->setIcon(QIcon(":/espina/note.svg"));
  connect(noteAction, SIGNAL(triggered(bool)),
          this, SLOT(addNote()));
}

//------------------------------------------------------------------------
void DefaultContextualMenu::createChangeCategoryMenu()
{
   auto changeCategoryMenu = new QMenu(tr("Change Category"));
   auto categoryListAction = new QWidgetAction(changeCategoryMenu);

   auto model = getModel();

   m_classification = new QTreeView();
   m_classification->header()->setVisible(false);
   m_classification->setModel(model.get());
   m_classification->setRootIndex(model->classificationRoot());
   m_classification->expandAll();

   connect(model.get(), SIGNAL(modelReset()),
           this,        SLOT(resetRootItem()));

   connect(m_classification, SIGNAL(clicked(QModelIndex)),
           this, SLOT(changeSegmentationsCategory(QModelIndex)));

   categoryListAction->setDefaultWidget(m_classification);
   changeCategoryMenu->addAction(categoryListAction);
   addMenu(changeCategoryMenu);
}

//------------------------------------------------------------------------
void DefaultContextualMenu::createTagsEntry()
{
  auto action = addAction(tr("Tags"));
  action->setIcon(QIcon(":/espina/tag.svg"));

  connect(action, SIGNAL(triggered(bool)),
          this,       SLOT(manageTags()));

}

//------------------------------------------------------------------------
void DefaultContextualMenu::createRenameEntry()
{
  auto action = addAction(tr("&Rename"));
  connect(action, SIGNAL(triggered(bool)),
          this,   SLOT(renameSegmentation()));
}


//------------------------------------------------------------------------
void DefaultContextualMenu::createExportEntry()
{
  auto action = addAction(tr("Export"));

  connect(action, SIGNAL(triggered(bool)),
          this,   SLOT(exportSelectedSegmentations()));
}

//------------------------------------------------------------------------
void DefaultContextualMenu::createDeleteEntry()
{
  auto action = addAction(tr("Delete"));

  connect(action, SIGNAL(triggered(bool)),
          this,   SLOT(deleteSelectedSementations()));
}
