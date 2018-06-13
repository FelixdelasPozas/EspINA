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
#include "SegmentationExplorerLayout.h"
#include <Dialogs/SegmentationInspector/SegmentationInspector.h>
#include <Extensions/Tags/SegmentationTags.h>
#include <Extensions/ExtensionUtils.h>
#include <Extensions/Notes/SegmentationNotes.h>
#include <GUI/Model/Utils/SegmentationUtils.h>
#include <Undo/RemoveSegmentations.h>

// Qt
#include <QUndoStack>

using namespace ESPINA;
using namespace ESPINA::Extensions;
using namespace ESPINA::GUI::Model::Utils;

const QString SegmentationExplorer::Layout::SEGMENTATION_MESSAGE  = QObject::tr("Deleting %1.\nDo you want to also delete the segmentations that compose it?");
const QString SegmentationExplorer::Layout::RECURSIVE_MESSAGE     = QObject::tr("Delete %1's segmentations. "
                                                                                "If you want to delete recursively select: Yes To All");
const QString SegmentationExplorer::Layout::MIXED_MESSAGE         = QObject::tr("Delete recursively %1's segmentations");

//------------------------------------------------------------------------
SegmentationFilterProxyModel::SegmentationFilterProxyModel(QObject *parent)
: QSortFilterProxyModel{parent}
{
  setFilterCaseSensitivity(Qt::CaseInsensitive);
}

//------------------------------------------------------------------------
bool SegmentationFilterProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
  bool acceptRows = QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);

  auto rowIndex = sourceModel()->index(source_row, 0, source_parent);

  if (!acceptRows)
  {
    auto item = itemAdapter(rowIndex);
    if (isSegmentation(item))
    {
      auto segmentation = segmentationPtr(item);
      auto extensions   = segmentation->readOnlyExtensions();

      if (extensions->hasExtension(SegmentationTags::TYPE))
      {
        auto tagExtension = retrieveExtension<SegmentationTags>(extensions);

        QStringList tags = tagExtension->tags();

        int i = 0;
        while (!acceptRows && i < tags.size())
        {
          acceptRows = tags[i].contains(filterRegExp());
          ++i;
        }
      }

      if(!acceptRows && extensions->hasExtension(SegmentationNotes::TYPE))
      {
        auto notesExtension = retrieveExtension<SegmentationNotes>(extensions);

        acceptRows = notesExtension->notes().contains(filterRegExp());
      }
    }
  }

  int row = 0;
  while (!acceptRows && row < sourceModel()->rowCount(rowIndex))
  {
    acceptRows = filterAcceptsRow(row, rowIndex);
    ++row;
  }

  return acceptRows;
}

//------------------------------------------------------------------------
SegmentationExplorer::Layout::Layout(CheckableTreeView              *view,
                                     Support::Context               &context)
: WithContext      (context)
, m_view           (view)
, m_active         {false}
{
}

//------------------------------------------------------------------------
SegmentationExplorer::Layout::~Layout()
{
  reset();
}

//------------------------------------------------------------------------
void SegmentationExplorer::Layout::deleteSegmentations(SegmentationAdapterList segmentations)
{
  if(!segmentations.empty())
  {
    auto undoStack = getUndoStack();
    auto names     = segmentationListNames(segmentations);

    undoStack->beginMacro(tr("Delete segmentation%1 %2.").arg(segmentations.size() > 1 ? "s":"").arg(names));
    undoStack->push(new RemoveSegmentations(segmentations, getModel()));
    undoStack->endMacro();
  }
}

//------------------------------------------------------------------------
void SegmentationExplorer::Layout::showSegmentationProperties(SegmentationAdapterList segmentations)
{
  if(!segmentations.empty())
  {
    auto inspector = m_inspectors.value(toKey(segmentations));
    if (!inspector)
    {
      inspector = new SegmentationInspector(segmentations, getContext());

      connect(inspector, SIGNAL(inspectorClosed(SegmentationInspector*)),
              this,      SLOT(releaseInspectorResources(SegmentationInspector*)), Qt::DirectConnection);

      connect(inspector, SIGNAL(segmentationsUpdated()),
              this,      SLOT(onInspectorUpdated()));

      m_inspectors.insert(toKey(segmentations), inspector);
    }
    inspector->show();
    inspector->raise();
  }
}

//------------------------------------------------------------------------
QModelIndexList SegmentationExplorer::Layout::indices(const QModelIndex& index, bool recursive)
{
  QModelIndexList res;

  if(model() != index.model())
  {
    qWarning() << "SegmentationExplorer::Layout::indices() -> Invalid QModelIndex model.";
  }
  else
  {
    for(int r = 0; r < model()->rowCount(index); r++)
    {
      auto child = index.child(r, 0);

      res << child;

      if (recursive)
      {
        res << indices(child, recursive);
      }
    }
  }

  return res;
}

//------------------------------------------------------------------------
void SegmentationExplorer::Layout::releaseInspectorResources(SegmentationInspector *inspector)
{
  auto key = m_inspectors.key(inspector);
  m_inspectors[key] = nullptr;
  m_inspectors.remove(key);
}

//------------------------------------------------------------------------
QString SegmentationExplorer::Layout::toKey(SegmentationAdapterList segmentations)
{
  QStringList pointers;
  QString result;

  for(auto seg : segmentations)
  {
    pointers += QString().number(reinterpret_cast<unsigned long long>(seg));
  }

  pointers.sort(); // O(n log n).

  for(auto pointer: pointers)
  {
    result += pointer + QString("|");
  }

  return result;
}

//------------------------------------------------------------------------
void SegmentationExplorer::Layout::reset()
{
  for(auto key: m_inspectors.keys())
  {
    m_inspectors[key]->close();
    m_inspectors.remove(key);
  }
}

//------------------------------------------------------------------------
QString SegmentationExplorer::Layout::toKey(SegmentationAdapterPtr segmentation)
{
  return QString("%1|").arg(reinterpret_cast<unsigned long long>(segmentation));
}

//------------------------------------------------------------------------
void SegmentationExplorer::Layout::onInspectorUpdated()
{
  auto inspector = qobject_cast<SegmentationInspector *>(sender());

  if(inspector)
  {
    auto iKey = m_inspectors.key(inspector);
    m_inspectors.remove(iKey);
    m_inspectors.insert(toKey(inspector->segmentations()), inspector);
  }
}

//------------------------------------------------------------------------
ItemAdapterPtr SegmentationExplorer::Layout::item(const QModelIndex &index) const
{
  if(!index.isValid() || !isActive()) return nullptr;

  return itemAdapter(index);
}
