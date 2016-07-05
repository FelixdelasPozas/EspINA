/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2015  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "TagUtils.h"

#include <Extensions/ExtensionUtils.h>
#include <GUI/Widgets/TagSelector.h>
#include <Extensions/Tags/SegmentationTags.h>
#include <Undo/ChangeSegmentationTags.h>

using namespace ESPINA;
using namespace ESPINA::Extensions;
using namespace ESPINA::Support::Utils;

//------------------------------------------------------------------------
QString dialogTitle(SegmentationAdapterList segmentations)
{
  auto title = segmentations[0]->data().toString();

  if (segmentations.size() > 1)
  {
    title.append(", " + segmentations[1]->data().toString());
  }

  if (segmentations.size() > 2)
  {
    title.append(", ...");
  }

  return title;
}
//------------------------------------------------------------------------
void Tags::manageTags(SegmentationAdapterList segmentations, QUndoStack *undoStack, ModelFactory *factory)
{
  if (!segmentations.isEmpty())
  {
    Q_ASSERT(undoStack);

    QStandardItemModel tags;

    QMap<QString, int> tagOcurrence;

    for(auto segmentation: segmentations)
    {
      auto extensions = segmentation->extensions();
      if(extensions->hasExtension(SegmentationTags::TYPE))
      {
        for(auto tag: extensions->get<SegmentationTags>()->tags())
        {
          tagOcurrence[tag] += 1;
        }
      }
    }

    for(auto tag: SegmentationTags::availableTags())
    {
      Qt::CheckState state;
      if(tagOcurrence[tag] == segmentations.size())
      {
        state = Qt::Checked;
      }
      else
      {
        if(tagOcurrence[tag] == 0)
        {
          state = Qt::Unchecked;
        }
        else
        {
          state = Qt::PartiallyChecked;
        }
      }
      auto item = new QStandardItem(tag);
      item->setCheckable(true);
      item->setCheckState(state);

      tags.appendRow(item);
    }
    tags.sort(0);

    QList<QUndoCommand *> commands;

    TagSelector tagSelector(dialogTitle(segmentations), tags);
    if (tagSelector.exec())
    {
      for(auto segmentation : segmentations)
      {
        QStringList currentTags, previousTags;

        {
          auto extensions = segmentation->readOnlyExtensions();

          if (extensions->hasExtension(SegmentationTags::TYPE))
          {
            currentTags  = extensions->get<SegmentationTags>()->tags();
            previousTags = currentTags;
          }
        }

        for (int i = 0; i < tags.rowCount(); ++i)
        {
          QStandardItem *item = tags.item(i);
          switch(item->checkState())
          {
            case Qt::Checked:
              if (!currentTags.contains(item->text()))
              {
                currentTags << item->text();
              }
              break;
            case Qt::Unchecked:
              if (currentTags.contains(item->text()))
              {
                currentTags.removeAll(item->text());
              }
              break;
            case Qt::PartiallyChecked:
              // unchanged.
            default:
              break;
          }
        }

        if (previousTags.toSet() != currentTags.toSet())
        {
          commands << new ChangeSegmentationTags(segmentation, currentTags, factory);
        }
      }
    }

    if (!commands.isEmpty())
    {
      undoStack->beginMacro(QObject::tr("Change Segmentation Tags"));
      for (auto command : commands)
      {
        undoStack->push(command);
      }
      undoStack->endMacro();
    }
  }
}

