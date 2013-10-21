/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  Jorge Peña Pastor <jpena@cesvima.upm.es>

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


#include "SegmentationContextualMenu.h"

#include <Core/Model/EspinaModel.h>
#include <Core/Model/Taxonomy.h>
#include <Core/Model/Segmentation.h>
#include <Core/Extensions/Tags/TagExtension.h>
#include <Core/Extensions/Notes/SegmentationNotes.h>
#include "GUI/QtWidget/NoteEditor.h"
#include "GUI/QtWidget/TagSelector.h"
#include <GUI/QtWidget/SegmentationVisualizationSettingsDialog.h>
#include <GUI/ViewManager.h>
#include <GUI/Representations/GraphicalRepresentationSettings.h>
#include <GUI/Representations/GraphicalRepresentation.h>
#include <GUI/Extensions/Visualization/VisualizationState.h>

#include <Undo/ChangeTaxonomyCommand.h>
#include <Undo/RemoveSegmentation.h>
#include <Undo/ChangeSegmentationTags.h>
#include <Undo/ChangeSegmentationNotes.h>

#include <QWidgetAction>
#include <QTreeView>
#include <QHeaderView>
#include <QStringList>
#include <QStringListModel>
#include <QListView>
#include <qinputdialog.h>
#include <QStandardItemModel>
#include <QMessageBox>

using namespace EspINA;

//------------------------------------------------------------------------
DefaultContextualMenu::DefaultContextualMenu(SegmentationList selection,
                                             ModelAdapter     *model,
                                             QUndoStack      *undoStack,
                                             ViewManager     *viewManager,
                                             QWidget         *parent)
: SegmentationContextualMenu(parent     )
, m_model        (model      )
, m_undoStack    (undoStack  )
, m_viewManager  (viewManager)
, m_segmentations(selection  )
{
  createChangeTaxonomyMenu();
  createNoteEntry();
  createTagsEntry();
  //createSetLevelOfDetailEntry();
  createRenameEntry();
  createVisualizationEntry();
}

//------------------------------------------------------------------------
void DefaultContextualMenu::addNote()
{
  if (!m_segmentations.isEmpty())
  {
    m_undoStack->beginMacro("Segmentation Notes");
  }

  foreach (SegmentationPtr segmentation, m_segmentations)
  {
    SegmentationNotes *notesExtension = dynamic_cast<SegmentationNotes *>(segmentation->informationExtension(SegmentationNotesID));

    NoteEditor editor(segmentation->data().toString(), notesExtension->note());

    if (editor.exec())
    {
      //notesExtension->setNote(editor.text());
      m_undoStack->push(new ChangeSegmentationNotes(notesExtension, editor.text()));
    }
  }

  if (!m_segmentations.isEmpty())
  {
    m_undoStack->endMacro();
  }

}

//------------------------------------------------------------------------
void DefaultContextualMenu::changeSegmentationsTaxonomy(const QModelIndex& index)
{
  this->hide();

  ModelItemPtr taxItem = indexPtr(index);
  Q_ASSERT(EspINA::TAXONOMY == taxItem->type());

  TaxonomyElementPtr taxonomy = taxonomyElementPtr(taxItem);

  m_undoStack->beginMacro(tr("Change Category"));
  {
    m_undoStack->push(new ChangeTaxonomyCommand(m_segmentations,
                                                taxonomy,
                                                m_model,
                                                m_viewManager));
  }
  m_undoStack->endMacro();

  emit changeTaxonomy(taxonomy);
}

//------------------------------------------------------------------------
void DefaultContextualMenu::changeFinalFlag()
{
  this->hide();

  bool value = m_changeFinalNode->isChecked();

  SegmentationList selectedSegmentations = m_segmentations;
  SegmentationList dependentSegmentations;
  SegmentationList rootSegmentations;

  foreach(SegmentationPtr seg, selectedSegmentations)
  {
    seg->setFinalNode(value);
    seg->setDependentNode(false);
    if (value)
      seg->setHierarchyRenderingType(HierarchyItem::Opaque, true);
    else
      seg->setHierarchyRenderingType(HierarchyItem::Undefined, false);

    foreach(SegmentationSPtr ancestor, seg->componentOf())
      rootSegmentations << ancestor.get();

    foreach(SegmentationSPtr successor, seg->components())
      dependentSegmentations << successor.get();
  }

  foreach(SegmentationPtr seg, dependentSegmentations)
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

    foreach(SegmentationSPtr successor, seg->components())
      dependentSegmentations << successor.get();
  }

  foreach(SegmentationPtr seg, rootSegmentations)
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

  // FIXME
//   foreach(SegmentationPtr seg, selectedSegmentations)
//     seg->output()->update();

  m_viewManager->updateSegmentationRepresentations(selectedSegmentations);
  m_viewManager->updateViews();

  emit changeFinalNode(value);
}

//------------------------------------------------------------------------
void DefaultContextualMenu::manageTags()
{
  QList<QUndoCommand *> pendingCommands;

  if (!m_segmentations.isEmpty())
  {
    QStandardItemModel tags;
    foreach (QString tag, SegmentationTags::TagModel.stringList())
    {
      QStandardItem *item = new QStandardItem(tag.toLower());

      SegmentationTags *tagExtension = SegmentationTags::extension(m_segmentations[0]);

      bool checked = tagExtension->tags().contains(tag);
      Qt::CheckState checkState = checked?Qt::Checked:Qt::Unchecked;

      int i = 1;
      while (checkState != Qt::PartiallyChecked && i < m_segmentations.size())
      {
        tagExtension = SegmentationTags::extension(m_segmentations[i]);
        if (checked != tagExtension->tags().contains(tag))
          checkState = Qt::PartiallyChecked;
        i++;
      }

      item->setCheckState(checkState);
      item->setCheckable(true);

      tags.appendRow(item);
    }

    TagSelector tagSelector(dialogTitle(), tags);
    if (tagSelector.exec())
    {
      foreach (SegmentationPtr segmentation, m_segmentations)
      {
        SegmentationTags *tagExtension = SegmentationTags::extension(segmentation);
        QStringList segTags = tagExtension->tags();
        QStringList oldTags = tagExtension->tags();

        for (int r = 0; r < tags.rowCount(); ++r)
        {
          QStandardItem *item = tags.item(r);
          if (Qt::Checked == item->checkState())
          {
            if (!segTags.contains(item->text()))
              segTags << item->text();
          } else if (Qt::Unchecked == item->checkState())
          {
            segTags.removeAll(item->text());
          }
        }
        if (oldTags != segTags)
          pendingCommands << new ChangeSegmentationTags(tagExtension, segTags);
      }
    }

  }

  if (!pendingCommands.isEmpty())
  {
    m_undoStack->beginMacro("Change Segmentation Tags");
    foreach (QUndoCommand *command, pendingCommands)
    {
      m_undoStack->push(command);
    }
    m_undoStack->endMacro();
  }
}

//------------------------------------------------------------------------
void DefaultContextualMenu::resetRootItem()
{
  m_taxonomyList->setRootIndex(m_model->taxonomyRoot());
}

//------------------------------------------------------------------------
void DefaultContextualMenu::renameSegmentation()
{
  foreach (SegmentationPtr segmentation, m_segmentations)
  {
    QString oldName = segmentation->data().toString();
    QString alias = QInputDialog::getText(this, oldName, "Rename Segmentation", QLineEdit::Normal, oldName);

    bool exists = false;
    foreach (SegmentationSPtr existinSegmentation, m_model->segmentations())
    {
      exists |= (existinSegmentation->data().toString() == alias && segmentation != existinSegmentation.get());
    }

    if (exists)
    {
      QMessageBox::warning(this,
                           tr("Alias duplicated"),
                           tr("Segmentation alias is already used by another segmentation."));
    } else
      segmentation->setData(alias, Qt::EditRole);
    // TODO: Undoable
  }
}

QStandardItem *findRepresentationItem(QList<QStandardItem *> items, GraphicalRepresentationSPtr representation)
{
  QStandardItem *representationItem = NULL;

  int i = 0;
  while (!representationItem && i < items.size())
  {
    QStandardItem *item = items[i];
    if (item->data(Qt::DisplayRole).toString() == representation->label())
    {
      representationItem = item;
    }
    ++i;
  }

  return representationItem;
}
//------------------------------------------------------------------------
void DefaultContextualMenu::displayVisualizationSettings()
{
  if (!m_segmentations.isEmpty())
  {
    SegmentationVisualizationSettingsDialog::Settings representations;

    foreach (SegmentationPtr segmentation, m_segmentations)
    {
      foreach (GraphicalRepresentationSPtr representation, segmentation->output()->graphicalRepresentations())
      {
        QStandardItem *representationItem = findRepresentationItem(representations.keys(), representation);

        if (!representationItem)
        {
          representationItem = new QStandardItem(representation->label());
          representationItem->setCheckState(representation->isActive()?Qt::Checked:Qt::Unchecked);
          representationItem->setCheckable(true);

          representations[representationItem] = representation->settingsWidget();
        }
        else
        {
          Qt::CheckState checkState = representation->isActive()?Qt::Checked:Qt::Unchecked;
          if (representationItem->checkState() != checkState)
            representationItem->setCheckState(Qt::PartiallyChecked);
        }
        representations[representationItem]->Get(representation);
      }
    }

    SegmentationVisualizationSettingsDialog visualizationSettings(dialogTitle(), representations);
    if (visualizationSettings.exec())
    {
      foreach (SegmentationPtr segmentation, m_segmentations)
      {
        VisualizationState *extension = dynamic_cast<VisualizationState *>(segmentation->informationExtension(VisualizationStateID));
        if (!extension)
        {
          extension = new VisualizationState();
          segmentation->addExtension(extension);
        }

        foreach (GraphicalRepresentationSPtr representation, segmentation->output()->graphicalRepresentations())
        {
          QStandardItem *key = findRepresentationItem(representations.keys(), representation);
          if (key->checkState() == Qt::Unchecked)
            representation->setActive(false);
          else if (key->checkState() == Qt::Checked)
          {
            representation->setActive(true);
            representations[key]->Set(representation);
          }
          extension->setSettings(representation->label(), representation->serializeSettings());
        }
      }
    }

  }

}

//------------------------------------------------------------------------
void DefaultContextualMenu::deleteSelectedSementations()
{
  this->hide();

  m_undoStack->beginMacro("Delete Segmentations");
  m_undoStack->push(new RemoveSegmentation(m_segmentations, m_model, m_viewManager));
  m_undoStack->endMacro();

  emit deleteSegmentations();
}

//------------------------------------------------------------------------
void DefaultContextualMenu::setSelection(SegmentationList list)
{
  this->m_segmentations = list;
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
void DefaultContextualMenu::createChangeTaxonomyMenu()
{
  QMenu         *changeTaxonomyMenu = new QMenu(tr("Change Category"));
  QWidgetAction *taxonomyListAction = new QWidgetAction(changeTaxonomyMenu);

  m_taxonomyList = new QTreeView();

  m_taxonomyList->header()->setVisible(false);
  m_taxonomyList->setModel(m_model);
  m_taxonomyList->setRootIndex(m_model->taxonomyRoot());
  m_taxonomyList->expandAll();
  connect(m_model, SIGNAL(modelReset()),
          this,  SLOT(resetRootItem()));
  connect(m_taxonomyList, SIGNAL(clicked(QModelIndex)),
          this, SLOT(changeSegmentationsTaxonomy(QModelIndex)));

  taxonomyListAction->setDefaultWidget(m_taxonomyList);
  changeTaxonomyMenu->addAction(taxonomyListAction);
  addMenu(changeTaxonomyMenu);
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
  m_changeFinalNode = this->addAction(tr("Modify level of detail"));
  m_changeFinalNode->setCheckable(true);
  connect(m_changeFinalNode, SIGNAL(triggered()),
          this, SLOT(changeFinalFlag()));

  QAction *deleteSegs = this->addAction(tr("Delete"));
  deleteSegs->setIcon(QIcon(":espina/trash-full.svg"));
  connect (deleteSegs, SIGNAL(triggered(bool)),
           this, SLOT(deleteSelectedSementations()));

  bool enabled = false;
  SegmentationList ancestors, successors;
  foreach (SegmentationPtr seg, m_segmentations)
  {
    enabled |= seg->IsFinalNode();
    foreach(SegmentationSPtr ancestor, seg->componentOf())
      ancestors <<  ancestor.get();
    foreach(SegmentationSPtr successor, seg->components())
      successors << successor.get();
  }

  foreach(SegmentationPtr seg, ancestors)
  {
    if (m_segmentations.contains(seg))
    {
      ancestors.removeAll(seg);
      break;
    }
    m_segmentations.append(seg);
    foreach(SegmentationSPtr ancestor, seg->componentOf())
      ancestors <<  ancestor.get();

    enabled |= seg->IsFinalNode();
  }

  foreach(SegmentationPtr seg, successors)
  {
    if (m_segmentations.contains(seg))
    {
      successors.removeAll(seg);
      break;
    }
    m_segmentations.append(seg);
    foreach(SegmentationSPtr successor, seg->components())
      successors << successor.get();

    enabled |= seg->IsFinalNode();
  }

  m_changeFinalNode->setChecked(enabled);
}

//------------------------------------------------------------------------
void DefaultContextualMenu::createRenameEntry()
{
  QAction *action = addAction(tr("&Rename"));
  //action->setShortcut("F2");

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
