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
#include <GUI/Widgets/Styles.h>
#include <Undo/ChangeSegmentationNotes.h>
#include <Undo/ChangeCategoryCommand.h>
#include <Undo/RenameSegmentationsCommand.h>
#include <Undo/RemoveSegmentations.h>
#include <Support/Utils/TagUtils.h>

#include <itkLabelObject.h>
#include <itkLabelMap.h>
#include <itkBinaryImageToLabelMapFilter.h>
#include <itkLabelMapToLabelImageFilter.h>
#include <itkMergeLabelMapFilter.h>

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
#include <QApplication>

using namespace ESPINA;
using namespace ESPINA::GUI;
using namespace ESPINA::GUI::Widgets::Styles;

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

template<typename T>
void exportSegmentations(ChannelAdapterPtr channel, SegmentationAdapterList &segmentations, const QString &file)
{
  using Label      = itk::LabelObject<T, 3>;
  using LabelMap   = itk::LabelMap<Label>;
  using LabelImage = itk::Image<T, 3>;

  using MergFilter = itk::MergeLabelMapFilter<LabelMap>;

  auto origin  = channel->position();
  auto spacing = channel->output()->spacing();

  auto labelMap = LabelMap::New();
  labelMap->SetSpacing(ItkSpacing<LabelMap>(spacing));
  labelMap->SetRegions(equivalentRegion<LabelMap>(origin, spacing, channel->bounds()));
  labelMap->Allocate();

  T i = 1;
  for (auto segmentation : segmentations)
  {
    auto volume = readLockVolume(segmentation->output())->itkImage();

    auto segLabelMapFilter = itk::BinaryImageToLabelMapFilter<itkVolumeType, LabelMap>::New();
    segLabelMapFilter->SetInput(volume);
    segLabelMapFilter->SetInputForegroundValue(SEG_VOXEL_VALUE);
    segLabelMapFilter->FullyConnectedOff();
    segLabelMapFilter->Update();

    auto segLabelMap = segLabelMapFilter->GetOutput();

    auto label = segLabelMap->GetNthLabelObject(0);

    // BinaryImageToLabelMapFilter create different labels for non connected components:
    // Merge disconnected components into first label object
    for (int n = 1; n <segLabelMap->GetNumberOfLabelObjects(); ++n)
    {
      auto part = segLabelMap->GetNthLabelObject(n);
      for (int l = 0; l < part->GetNumberOfLines(); ++l)
      {
        label->AddLine(part->GetLine(l));
      }
    }
    label->SetLabel(i++);
    labelMap->AddLabelObject(label);
  }
  //qDebug() << "Total labels" << labelMap->GetNumberOfLabelObjects();

  auto image = itk::LabelMapToLabelImageFilter<LabelMap, LabelImage>::New();

  image->SetInput(labelMap);
  image->SetNumberOfThreads(1);

  try
  {
    image->Update();

    exportVolume<LabelImage>(image->GetOutput(), file);
  }
  catch(itk::MemoryAllocationError &e)
  {
    auto title = QObject::tr("Export Segmentation Binarization");
    auto msg   = QObject::tr("Insufficient memory to export selected segmentations.");

    if (segmentations.size() >= 256)
    {
      msg.append(QObject::tr("\nTry exporting less than 256 segmentations to produce 8 bit labelmaps instead of 16 bit ones"));
    }

    DefaultDialogs::InformationMessage(title, msg);
  }
}

//------------------------------------------------------------------------
void DefaultContextualMenu::exportSelectedSegmentations()
{
  auto title  = tr("Export selected segmentations");
  auto format = SupportedFormats().addFormat(tr("Binary Stack"), "tif");

  auto file = DefaultDialogs::SaveFile(title, format);

  if (file.isEmpty()) return;

  WaitingCursor cursor;

  if (m_segmentations.size() < 256)
  {
    exportSegmentations<unsigned char>(getActiveChannel(), m_segmentations, file);
  }
  else
  {
    exportSegmentations<unsigned short>(getActiveChannel(), m_segmentations, file);
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
  auto action = addAction(tr("Notes"));

  action->setIcon(QIcon(":/espina/note.svg"));
  connect(action, SIGNAL(triggered(bool)),
          this,   SLOT(addNote()));
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
