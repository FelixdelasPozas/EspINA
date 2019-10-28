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
#include <App/Utils/TagUtils.h>
#include <App/Dialogs/ColorEngineSelector/ColorEngineSelector.h>
#include <Core/Utils/SupportedFormats.h>
#include <Core/Utils/ListUtils.hxx>
#include <Core/Utils/vtkPolyDataUtils.h>
#include <Core/Analysis/Data/MeshData.h>
#include <Core/Analysis/Data/SkeletonData.h>
#include <Extensions/ExtensionUtils.h>
#include <Extensions/Notes/SegmentationNotes.h>
#include <GUI/Widgets/NoteEditor.h>
#include <GUI/Widgets/Styles.h>
#include <GUI/Dialogs/DefaultDialogs.h>
#include <GUI/Model/Utils/SegmentationUtils.h>
#include <Undo/ChangeSegmentationNotes.h>
#include <Undo/RenameSegmentationsCommand.h>
#include <Undo/RemoveSegmentations.h>

// ITK
#include <itkLabelObject.h>
#include <itkLabelMap.h>
#include <itkBinaryImageToLabelMapFilter.h>
#include <itkLabelMapToLabelImageFilter.h>
#include <itkMergeLabelMapFilter.h>

// Qt
#include <QHeaderView>
#include <QStringList>
#include <QStringListModel>
#include <QListView>
#include <QInputDialog>
#include <QStandardItemModel>
#include <QMessageBox>
#include <QApplication>

using namespace ESPINA;
using namespace ESPINA::Extensions;
using namespace ESPINA::Core::Utils;
using namespace ESPINA::GUI;
using namespace ESPINA::GUI::View;
using namespace ESPINA::GUI::Model::Utils;
using namespace ESPINA::GUI::Widgets::Styles;

//------------------------------------------------------------------------
DefaultContextualMenu::DefaultContextualMenu(SegmentationAdapterList selection,
                                             Support::Context       &context,
                                             QWidget                *parent)
: ContextualMenu {parent}
, WithContext    (context)
, m_segmentations{selection}
{
  createNoteEntry();
  createTagsEntry();
  createRenameEntry();
  createGroupRenameEntry();
  createExportEntry();
  createDeleteEntry();
  createColorEntry();

  // to comment in production
  // createFixesEntry();
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

    if (QDialog::Accepted == editor.exec())
    {
      commands << new ChangeSegmentationNotes(segmentation, editor.text(), getContext().factory().get());
    }
  }

  if (!commands.isEmpty())
  {
    auto undoStack = getUndoStack();
    auto names     = segmentationListNames(m_segmentations);

    WaitingCursor cursor;

    undoStack->beginMacro(tr("Add notes to %1.").arg(names));
    for (auto command : commands)
    {
      undoStack->push(command);
    }
    undoStack->endMacro();
  }
}

//------------------------------------------------------------------------
void DefaultContextualMenu::manageTags()
{
  manageTagsDialog(m_segmentations, getUndoStack(), getContext().factory().get());
}

//------------------------------------------------------------------------
void DefaultContextualMenu::renameSegmentation()
{
  QMap<SegmentationAdapterPtr, QString> renames;
  QString names;

  for (auto segmentation : m_segmentations)
  {
    bool result = false;
    auto oldName = segmentation->data().toString();
    auto newName = QInputDialog::getText(DefaultDialogs::defaultParentWidget(), oldName, tr("Rename segmentation"), QLineEdit::Normal, oldName, &result);
    QString prefix = (segmentation == m_segmentations.first() ? "":(segmentation == m_segmentations.last() ? " and ":", "));

    if(!result) continue;

    bool exists = false;
    for (auto existinSegmentation : getModel()->segmentations())
    {
      exists |= (existinSegmentation->data().toString() == newName);
    }

    if (exists)
    {
      auto title   = tr("Name duplicated");
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
        names += tr("%1'%2' to '%3'").arg(prefix).arg(oldName).arg(newName);
      }
    }
  }

  if (renames.size() != 0)
  {
    WaitingCursor cursor;

    auto undoStack = getUndoStack();
    undoStack->beginMacro(tr("Rename segmentation%1: %2.").arg(m_segmentations.size() > 1 ? "s":"").arg(names));
    undoStack->push(new RenameSegmentationsCommand(renames));
    undoStack->endMacro();
  }
}

//------------------------------------------------------------------------
void DefaultContextualMenu::renameSegmentationGroup()
{
  QString names;
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
    QString oldName  = segmentation->data().toString();
    auto numIndex    = regExpr.indexIn(oldName);
    auto newName     = prefix + " " + oldName.mid(numIndex, oldName.length()-numIndex);
    auto namesPrefix = (segmentation == m_segmentations.first() ? "":(segmentation == m_segmentations.last() ? " and ":", "));

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
      names += tr("%1'%2' to '%3'").arg(namesPrefix).arg(segmentation->data().toString()).arg(newName);
    }
  }

  if (renames.size() != 0)
  {
    WaitingCursor cursor;

    auto undoStack = getUndoStack();
    undoStack->beginMacro(tr("Rename segmentations group: %1.").arg(names));
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

  if(!m_segmentations.isEmpty())
  {
    auto message  = QObject::tr("Do you really want to delete the selected segmentations?");
    auto title    = QObject::tr("Delete Selected Items");
    auto details  = QObject::tr("Selected to be deleted:");

    for(int i = 0; i < m_segmentations.size(); ++i)
    {
      details.append(QString("\n - %1").arg(m_segmentations.at(i)->data().toString()));
    }

    if(QMessageBox::Ok == GUI::DefaultDialogs::UserQuestion(message, QMessageBox::Cancel|QMessageBox::Ok, title, details))
    {
      auto undoStack = getUndoStack();
      auto names     = segmentationListNames(m_segmentations);

      {
        WaitingCursor cursor;

        undoStack->beginMacro(tr("Delete segmentation%1: %2.").arg(m_segmentations.size() > 1 ? "s":"").arg(names));
        undoStack->push(new RemoveSegmentations(m_segmentations, getModel()));
        undoStack->endMacro();
      }

      emit deleteSegmentations();
    }
  }
}

//------------------------------------------------------------------------
void DefaultContextualMenu::createNoteEntry()
{
  auto action = addAction(tr("&Notes..."));

  action->setIcon(QIcon(":/espina/note.svg"));
  connect(action, SIGNAL(triggered(bool)),
          this,   SLOT(addNote()));
}

//------------------------------------------------------------------------
void DefaultContextualMenu::createTagsEntry()
{
  auto action = addAction(tr("&Tags..."));
  action->setIcon(QIcon(":/espina/tag.svg"));

  connect(action, SIGNAL(triggered(bool)),
          this,       SLOT(manageTags()));

}

//------------------------------------------------------------------------
void DefaultContextualMenu::createRenameEntry()
{
  auto action = addAction(tr("&Rename..."));
  connect(action, SIGNAL(triggered(bool)),
          this,   SLOT(renameSegmentation()));
}

//------------------------------------------------------------------------
void DefaultContextualMenu::createGroupRenameEntry()
{
  auto action = addAction(tr("Rename &All..."));
  action->setEnabled(m_segmentations.size() > 1);

  connect(action, SIGNAL(triggered(bool)),
          this,   SLOT(renameSegmentationGroup()));
}

//------------------------------------------------------------------------
void DefaultContextualMenu::createExportEntry()
{
  auto action = addAction(tr("&Export to TIFF image..."));

  connect(action, SIGNAL(triggered(bool)),
          this,   SLOT(exportSelectedSegmentations()));

  action = addAction(tr("E&xport to Wavefront OBJ..."));
  action->setEnabled(m_segmentations.size() == 1);

  connect(action, SIGNAL(triggered(bool)),
          this,   SLOT(exportSegmentationToOBJ()));
}

//------------------------------------------------------------------------
void DefaultContextualMenu::changeSegmentationsColorEngine()
{
  this->hide();

  auto colorEngine = std::dynamic_pointer_cast<GUI::ColorEngines::MultiColorEngine>(getContext().colorEngine());
  if(colorEngine)
  {
    ColorEngineSelector dialog{colorEngine};
    if(dialog.exec() == QDialog::Accepted)
    {
      auto segColorEngine = dialog.colorEngine();

      for(auto segmentation: m_segmentations)
      {
        if(segColorEngine)
        {
          segmentation->setColorEngine(segColorEngine);
        }
        else
        {
          segmentation->clearColorEngine();
        }

        auto items = Core::Utils::toList<ViewItemAdapter>(m_segmentations);
        getViewState().invalidateRepresentationColors(items);
      }
    }
  }
}

//------------------------------------------------------------------------
void DefaultContextualMenu::createDeleteEntry()
{
  auto action = addAction(tr("&Delete..."));

  connect(action, SIGNAL(triggered(bool)),
          this,   SLOT(deleteSelectedSementations()));
}

//------------------------------------------------------------------------
void DefaultContextualMenu::createColorEntry()
{
  auto action = addAction(tr("&Custom coloring..."));

  connect(action, SIGNAL(triggered(bool)),
          this,   SLOT(changeSegmentationsColorEngine()));
}

//------------------------------------------------------------------------
void DefaultContextualMenu::exportSegmentationToOBJ()
{
  auto segmentation = m_segmentations.first();
  const auto name   = segmentation->data().toString();

  auto title      = tr("Export segmentation '%1'").arg(name);
  auto suggestion = tr("%1.obj").arg(name);
  auto format     = SupportedFormats().addFormat(tr("Wavefront OBJ file"), "obj");

  auto file = DefaultDialogs::SaveFile(title, format, QDir::homePath(), ".obj", suggestion);

  if (file.isEmpty()) return;

  WaitingCursor cursor;

  QByteArray buffer;

  if(hasMeshData(segmentation->output()))
  {
    auto data = readLockMesh(segmentation->output());
    buffer = PolyDataUtils::convertPolyDataToOBJ(data->mesh());
  }
  else
  {
    if(hasSkeletonData(segmentation->output()))
    {
      auto data = readLockSkeleton(segmentation->output());
      buffer = PolyDataUtils::convertPolyDataToOBJ(data->skeleton());
    }
    else
    {
      const auto message = tr("Segmentation '%1' has no mesh or skeleton data to be exported.").arg(name);
      DefaultDialogs::InformationMessage(title, message);
      return;
    }
  }

  QFile objFile(file);
  if(!objFile.open(QIODevice::Text|QIODevice::Truncate|QIODevice::WriteOnly))
  {
    const auto message = tr("Unable to create destination file '%1'.").arg(file);
    DefaultDialogs::InformationMessage(title, message);
    return;
  }

  objFile.write(buffer);
  objFile.flush();
  objFile.close();
}

//------------------------------------------------------------------------
void DefaultContextualMenu::createFixesEntry()
{
  auto action = addAction(tr("Apply &fixes"));

  connect(action, SIGNAL(triggered(bool)),
          this,   SLOT(doFixes()));
}

//------------------------------------------------------------------------
void DefaultContextualMenu::doFixes()
{
  // FIXES
}
