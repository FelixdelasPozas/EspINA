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
#include <Core/Utils/SupportedFormats.h>
#include <Extensions/ExtensionUtils.h>
#include <Extensions/Notes/SegmentationNotes.h>
#include <GUI/Widgets/NoteEditor.h>
#include <GUI/Widgets/Styles.h>
#include <Undo/ChangeSegmentationNotes.h>
#include <Undo/ChangeCategoryCommand.h>
#include <Undo/RenameSegmentationsCommand.h>
#include <Undo/RemoveSegmentations.h>
#include <Support/Utils/TagUtils.h>

// ITK
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
using namespace ESPINA::Core::Utils;
using namespace ESPINA::GUI;
using namespace ESPINA::GUI::View;
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
  createGroupRenameEntry();
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
    bool result = false;
    QString oldName = segmentation->data().toString();
    QString newName = QInputDialog::getText(DefaultDialogs::defaultParentWidget(), oldName, renameTitle, QLineEdit::Normal, oldName, &result);

    if(!result) continue;

    bool exists = false;
    for (auto existinSegmentation : getModel()->segmentations())
    {
      exists |= (existinSegmentation->data().toString() == newName);
    }

    if (exists)
    {
      auto title   = tr("Alias duplicated");
      auto msg     = tr("Segmentation name '%1' is already used by another segmentation.").arg(newName);
      auto buttons = QMessageBox::Abort|QMessageBox::Discard;

      if(QMessageBox::Abort == DefaultDialogs::UserQuestion(msg, buttons, title))
      {
        break;
      }
    }
    else
    {
      if(oldName != newName)
      {
        renames[segmentation] = newName;
      }
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
void DefaultContextualMenu::renameSegmentationGroup()
{
  auto renameTitle = tr("Rename Segmentations Group");

  QMap<SegmentationAdapterPtr, QString> renames;
  auto hint = m_segmentations.first()->data().toString();

  QRegExp regExpr{"[0-9]"};

  auto index = regExpr.indexIn(hint);

  if(index > 1)
  {
   hint = hint.mid(0, (hint[index-1] == ' ' ? index-1 : index));
  }

  bool result = false;
  QString prefix = QInputDialog::getText(DefaultDialogs::defaultParentWidget(), tr("Rename Group"), tr("Segmentations prefix"), QLineEdit::Normal, hint, &result);

  if(!result) return;

  for (auto segmentation : m_segmentations)
  {
    QString oldName = segmentation->data().toString();
    auto numIndex = regExpr.indexIn(oldName);
    auto newName = prefix + " " + oldName.mid(numIndex, oldName.length()-numIndex);

    bool exists = false;
    for (auto existinSegmentation : getModel()->segmentations())
    {
      exists |= (existinSegmentation->data().toString() == newName);
    }

    if (exists)
    {
      auto title   = tr("Alias duplicated");
      auto msg     = tr("Segmentation name '%1' is already used by another segmentation.").arg(newName);
      auto buttons = QMessageBox::Abort|QMessageBox::Discard;

      if(QMessageBox::Abort == DefaultDialogs::UserQuestion(msg, buttons, title))
      {
        break;
      }
    }
    else
    {
      renames.insert(segmentation, newName);
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
template<typename T>
void exportSegmentations(ChannelAdapterPtr channel, SegmentationAdapterList &segmentations, const QString &file)
{
  using Label            = itk::LabelObject<T, 3>;
  using LabelMap         = itk::LabelMap<Label>;
  using LabelImage       = itk::Image<T, 3>;
  using MergFilter       = itk::MergeLabelMapFilter<LabelMap>;
  using Binary2Label     = itk::BinaryImageToLabelMapFilter<itkVolumeType, LabelMap>;
  using Label2LabelImage = itk::LabelMapToLabelImageFilter<LabelMap, LabelImage>;

  auto origin  = channel->position();
  auto spacing = channel->output()->spacing();
  auto region  = equivalentRegion<LabelMap>(origin, spacing, channel->bounds());

  auto labelMap = LabelMap::New();
  labelMap->SetSpacing(ItkSpacing<LabelMap>(spacing));
  labelMap->SetRegions(region);
  labelMap->Allocate();

  T i = 1;
  QStringList croppedSegmentations;
  for (auto segmentation : segmentations)
  {
    auto volume = readLockVolume(segmentation->output())->itkImage();

    // NOTE: some segmentations could have voxels outside the stack bounds
    //       (a dilated segmentation in the edge of the stack for example)
    //       so a crop is needed in that case.
    auto imageRegion = volume->GetLargestPossibleRegion();
    if (!region.IsInside(imageRegion))
    {
      croppedSegmentations << segmentation->data().toString();

      imageRegion.Crop(region);

      volume = extract_image<itkVolumeType>(volume, imageRegion);
    }

    auto segLabelMapFilter = Binary2Label::New();
    segLabelMapFilter->SetInput(volume);
    segLabelMapFilter->SetInputForegroundValue(SEG_VOXEL_VALUE);
    segLabelMapFilter->FullyConnectedOff();
    segLabelMapFilter->Update();

    auto segLabelMap = segLabelMapFilter->GetOutput();

    auto label = segLabelMap->GetNthLabelObject(0);

    // BinaryImageToLabelMapFilter create different labels for non connected components:
    // Merge disconnected components into first label object
    for (unsigned int n = 1; n < segLabelMap->GetNumberOfLabelObjects(); ++n)
    {
      auto part = segLabelMap->GetNthLabelObject(n);
      for (unsigned int l = 0; l < part->GetNumberOfLines(); ++l)
      {
        label->AddLine(part->GetLine(l));
      }
    }
    label->SetLabel(i++);
    labelMap->AddLabelObject(label);
  }

  auto image = Label2LabelImage::New();

  image->SetInput(labelMap);
  image->SetNumberOfThreads(1);

  auto title = QObject::tr("Export Segmentation%1 to a Binary Stack").arg(segmentations.size() > 1 ? "s" : "");

  try
  {
    image->Update();

    exportVolume<LabelImage>(image->GetOutput(), file);
  }
  catch (const itk::MemoryAllocationError &e)
  {
    auto msg = QObject::tr("Insufficient memory to export selected segmentations.");

    if (segmentations.size() >= 256)
    {
      msg.append(QObject::tr("\nTry exporting less than 256 segmentations to produce 8 bit images instead of 16 bit ones"));
    }

    DefaultDialogs::InformationMessage(msg, title);
  }
  catch (const itk::ExceptionObject &e)
  {
    auto msg = QObject::tr("Error exporting: %1.").arg(QString(e.GetDescription()));

    DefaultDialogs::InformationMessage(msg, title);
  }

  if(!croppedSegmentations.isEmpty())
  {
    auto msg = QObject::tr("Several segmentations needed to be cropped to fit stack bounds.");
    auto details = QObject::tr("The following segmentations have voxels outside of the stack bounds and were cropped to fit:\n");
    for(auto name: croppedSegmentations)
    {
      details += QString("\n- %1").arg(name);
    }

    DefaultDialogs::InformationMessage(msg, title, details);
  }
}

//------------------------------------------------------------------------
void DefaultContextualMenu::exportSelectedSegmentations()
{
  auto title      = tr("Export selected segmentations");
  auto suggestion = tr("segmentation_images.tif");
  auto format     = SupportedFormats().addFormat(tr("Binary Stack"), "tif");

  auto file = DefaultDialogs::SaveFile(title, format, QDir::homePath(), ".tif", suggestion);

  if (file.isEmpty()) return;

  WaitingCursor cursor;

  if (m_segmentations.size() < 256)
  {
    exportSegmentations<unsigned char>(getActiveChannel(), m_segmentations, file);
  }
  else
  {
    if(m_segmentations.size() < 65535)
    {
      exportSegmentations<unsigned short>(getActiveChannel(), m_segmentations, file);
    }
    else // short and int are not guaranteed to be different in size, don't even check and go for the next one.
    {
      exportSegmentations<unsigned long>(getActiveChannel(), m_segmentations, file);
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
void DefaultContextualMenu::createNoteEntry()
{
  auto action = addAction(tr("&Notes"));

  action->setIcon(QIcon(":/espina/note.svg"));
  connect(action, SIGNAL(triggered(bool)),
          this,   SLOT(addNote()));
}

//------------------------------------------------------------------------
void DefaultContextualMenu::createChangeCategoryMenu()
{
   auto changeCategoryMenu = new QMenu(tr("C&hange Category"));
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
  auto action = addAction(tr("&Tags"));
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
void DefaultContextualMenu::createGroupRenameEntry()
{
  auto action = addAction(tr("Rename &All"));
  action->setEnabled(m_segmentations.size() > 1);

  connect(action, SIGNAL(triggered(bool)),
          this,   SLOT(renameSegmentationGroup()));
}

//------------------------------------------------------------------------
void DefaultContextualMenu::createExportEntry()
{
  auto action = addAction(tr("&Export"));

  connect(action, SIGNAL(triggered(bool)),
          this,   SLOT(exportSelectedSegmentations()));
}

//------------------------------------------------------------------------
void DefaultContextualMenu::createDeleteEntry()
{
  auto action = addAction(tr("&Delete"));

  connect(action, SIGNAL(triggered(bool)),
          this,   SLOT(deleteSelectedSementations()));
}
