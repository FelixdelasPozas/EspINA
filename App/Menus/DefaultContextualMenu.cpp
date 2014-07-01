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

#include <QWidgetAction>
#include <QTreeView>
#include <QHeaderView>
#include <QStringList>
#include <QStringListModel>
#include <QListView>
#include <QInputDialog>
#include <QStandardItemModel>
#include <QMessageBox>

using namespace EspINA;

//------------------------------------------------------------------------
DefaultContextualMenu::DefaultContextualMenu(SegmentationAdapterList selection,
                                             ModelAdapterSPtr        model,
                                             ViewManagerSPtr         viewManager,
                                             QUndoStack             *undoStack,
                                             QWidget                *parent)
: ContextualMenu(parent)
, m_model         {model}
, m_viewManager   {viewManager}
, m_undoStack     {undoStack}
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
    delete m_classification;
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
    m_undoStack->beginMacro(tr("Add Notes"));
    for (auto command : commands)
    {
      m_undoStack->push(command);
    }
    m_undoStack->endMacro();
  }
}

//------------------------------------------------------------------------
void DefaultContextualMenu::changeSegmentationsCategory(const QModelIndex& index)
{
   this->hide();

   ItemAdapterPtr categoryItem = itemAdapter(index);
   Q_ASSERT(isCategory(categoryItem));
   CategoryAdapterPtr categoryAdapter = categoryPtr(categoryItem);

   m_undoStack->beginMacro(tr("Change Category"));
   {
     m_undoStack->push(new ChangeCategoryCommand(m_segmentations,
                                                 categoryAdapter,
                                                 m_model,
                                                 m_viewManager));
   }
   m_undoStack->endMacro();

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
    m_undoStack->beginMacro(tr("Change Segmentation Tags"));
    for (auto command : commands)
    {
      m_undoStack->push(command);
    }
    m_undoStack->endMacro();
  }
}

//------------------------------------------------------------------------
void DefaultContextualMenu::resetRootItem()
{
  m_classification->setRootIndex(m_model->classificationRoot());
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
    for (auto existinSegmentation : m_model->segmentations())
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
    m_undoStack->beginMacro(QString("Rename segmentations"));
    m_undoStack->push(new RenameSegmentationsCommand(renames));
    m_undoStack->endMacro();
  }
}

//------------------------------------------------------------------------
// QStandardItem *findRepresentationItem(QList<QStandardItem *> items, RepresentationSPtr representation)
// {
//   QStandardItem *representationItem = NULL;
//
//   int i = 0;
//   while (!representationItem && i < items.size())
//   {
//     QStandardItem *item = items[i];
//     if (item->data(Qt::DisplayRole).toString() == representation->label())
//     {
//       representationItem = item;
//     }
//     ++i;
//   }
//
//   return representationItem;
// }

//------------------------------------------------------------------------
void DefaultContextualMenu::displayVisualizationSettings()
{
//   if (!m_segmentations.isEmpty())
//   {
//     SegmentationVisualizationSettingsDialog::Settings representations;
//
//     foreach (SegmentationPtr segmentation, m_segmentations)
//     {
//       foreach (GraphicalRepresentationSPtr representation, segmentation->output()->graphicalRepresentations())
//       {
//         QStandardItem *representationItem = findRepresentationItem(representations.keys(), representation);
//
//         if (!representationItem)
//         {
//           representationItem = new QStandardItem(representation->label());
//           representationItem->setCheckState(representation->isActive()?Qt::Checked:Qt::Unchecked);
//           representationItem->setCheckable(true);
//
//           representations[representationItem] = representation->settingsWidget();
//         }
//         else
//         {
//           Qt::CheckState checkState = representation->isActive()?Qt::Checked:Qt::Unchecked;
//           if (representationItem->checkState() != checkState)
//             representationItem->setCheckState(Qt::PartiallyChecked);
//         }
//         representations[representationItem]->Get(representation);
//       }
//     }
//
//     SegmentationVisualizationSettingsDialog visualizationSettings(dialogTitle(), representations);
//     if (visualizationSettings.exec())
//     {
//       foreach (SegmentationPtr segmentation, m_segmentations)
//       {
//         VisualizationState *extension = dynamic_cast<VisualizationState *>(segmentation->informationExtension(VisualizationStateID));
//         if (!extension)
//         {
//           extension = new VisualizationState();
//           segmentation->addExtension(extension);
//         }
//
//         foreach (GraphicalRepresentationSPtr representation, segmentation->output()->graphicalRepresentations())
//         {
//           QStandardItem *key = findRepresentationItem(representations.keys(), representation);
//           if (key->checkState() == Qt::Unchecked)
//             representation->setActive(false);
//           else if (key->checkState() == Qt::Checked)
//           {
//             representation->setActive(true);
//             representations[key]->Set(representation);
//           }
//           extension->setSettings(representation->label(), representation->serializeSettings());
//         }
//       }
//     }
//
//   }
}

//------------------------------------------------------------------------
void DefaultContextualMenu::deleteSelectedSementations()
{
  this->hide();

  m_undoStack->beginMacro("Delete Segmentations");
  m_undoStack->push(new RemoveSegmentations(m_segmentations, m_model));
  m_undoStack->endMacro();

  emit deleteSegmentations();
}

//------------------------------------------------------------------------
void DefaultContextualMenu::setSelection(SelectionSPtr selection)
{
  //this->m_segmentations = list;
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

   m_classification = new QTreeView();
   m_classification->header()->setVisible(false);
   m_classification->setModel(m_model.get());
   m_classification->setRootIndex(m_model->classificationRoot());
   m_classification->expandAll();
   connect(m_model.get(), SIGNAL(modelReset()),
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
          this, SLOT(manageTags()));

}

//------------------------------------------------------------------------
void DefaultContextualMenu::createSetLevelOfDetailEntry()
{
//   m_changeFinalNode = this->addAction(tr("Modify level of detail"));
//   m_changeFinalNode->setCheckable(true);
//   connect(m_changeFinalNode, SIGNAL(triggered()),
//           this, SLOT(changeFinalFlag()));
//
//   QAction *deleteSegs = this->addAction(tr("Delete"));
//   deleteSegs->setIcon(QIcon(":espina/trash-full.svg"));
//   connect (deleteSegs, SIGNAL(triggered(bool)),
//            this, SLOT(deleteSelectedSementations()));
//
//   bool enabled = false;
//   SegmentationList ancestors, successors;
//   foreach (SegmentationPtr seg, m_segmentations)
//   {
//     enabled |= seg->IsFinalNode();
//     foreach(SegmentationSPtr ancestor, seg->componentOf())
//       ancestors <<  ancestor.get();
//     foreach(SegmentationSPtr successor, seg->components())
//       successors << successor.get();
//   }
//
//   foreach(SegmentationPtr seg, ancestors)
//   {
//     if (m_segmentations.contains(seg))
//     {
//       ancestors.removeAll(seg);
//       break;
//     }
//     m_segmentations.append(seg);
//     foreach(SegmentationSPtr ancestor, seg->componentOf())
//       ancestors <<  ancestor.get();
//
//     enabled |= seg->IsFinalNode();
//   }
//
//   foreach(SegmentationPtr seg, successors)
//   {
//     if (m_segmentations.contains(seg))
//     {
//       successors.removeAll(seg);
//       break;
//     }
//     m_segmentations.append(seg);
//     foreach(SegmentationSPtr successor, seg->components())
//       successors << successor.get();
//
//     enabled |= seg->IsFinalNode();
//   }
//
//   m_changeFinalNode->setChecked(enabled);
}

//------------------------------------------------------------------------
void DefaultContextualMenu::createRenameEntry()
{
  QAction *action = addAction(tr("&Rename"));
  connect(action, SIGNAL(triggered(bool)),
          this, SLOT(renameSegmentation()));
}

//------------------------------------------------------------------------
void DefaultContextualMenu::createVisualizationEntry()
{
  QAction *action = addAction(tr("&Visualization"));

  connect(action, SIGNAL(triggered(bool)),
          this, SLOT(displayVisualizationSettings()));
}

//------------------------------------------------------------------------
QString DefaultContextualMenu::dialogTitle() const
{
  QString title = m_segmentations[0]->data().toString();
  if (m_segmentations.size() > 1)
    title.append(", " + m_segmentations[1]->data().toString());
  if (m_segmentations.size() > 2)
    title.append(", ...");

  return title;
}

//------------------------------------------------------------------------
void DefaultContextualMenu::createDeleteEntry()
{
  QAction *deleteAction = addAction(tr("Delete"));

  connect(deleteAction, SIGNAL(triggered(bool)),
          this,         SLOT(deleteSelectedSementations()));
}
