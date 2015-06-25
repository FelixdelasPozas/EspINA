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


#include "SegmentationExplorerLayout.h"
#include <Undo/RemoveSegmentations.h>

#include <Dialogs/SegmentationInspector/SegmentationInspector.h>
#include <Extensions/Tags/SegmentationTags.h>
#include <Extensions/ExtensionUtils.h>
#include <GUI/Model/Utils/SegmentationUtils.h>

// #include <Core/Model/Segmentation.h>
// #include <Core/Extensions/Tags/TagExtension.h>
// #include <Undo/RemoveSegmentation.h>
#include <QUndoStack>

using namespace ESPINA;
using namespace ESPINA::GUI::Model::Utils;

const QString SegmentationExplorer::Layout::SEGMENTATION_MESSAGE
  = QObject::tr("Deleting %1.\nDo you want to also delete the segmentations that compose it?");
const QString SegmentationExplorer::Layout::RECURSIVE_MESSAGE
  = QObject::tr("Delete %1's segmentations. "
                "If you want to delete recursively select: Yes To All");
const QString SegmentationExplorer::Layout::MIXED_MESSAGE
  = QObject::tr("Delete recursively %1's segmentations");

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
    ItemAdapterPtr item = itemAdapter(rowIndex);
    if (isSegmentation(item))
    {
      SegmentationAdapterPtr segmentation = segmentationPtr(item);

      if (segmentation->hasExtension(SegmentationTags::TYPE))
      {
        auto tagExtension = retrieveExtension<SegmentationTags>(segmentation);

        QStringList tags = tagExtension->tags();
        int i = 0;
        while (!acceptRows && i < tags.size())
        {
          acceptRows = tags[i].contains(filterRegExp());
          ++i;
        }
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
                                     Support::FilterRefinerRegister &filterRefiners,
                                     Support::Context               &context)
: m_context        (context)
, m_delegateFactory{filterRefiners}
, m_view           {view}
{
  connect(m_context.model().get(), SIGNAL(rowsAboutToBeRemoved(QModelIndex, int , int)),
          this, SLOT(rowsAboutToBeRemoved(QModelIndex, int,int)));
}

//------------------------------------------------------------------------
SegmentationExplorer::Layout::~Layout()
{
  reset();
}

//------------------------------------------------------------------------
void SegmentationExplorer::Layout::deleteSegmentations(SegmentationAdapterList segmentations)
{
  auto undoStack = m_context.undoStack();

  undoStack->beginMacro(tr("Delete Segmentations"));
  undoStack->push(new RemoveSegmentations(segmentations, m_context.model()));
  undoStack->endMacro();
}

//------------------------------------------------------------------------
void SegmentationExplorer::Layout::showSegmentationInformation(SegmentationAdapterList segmentations)
{
  auto inspector = m_inspectors.value(toKey(segmentations));
  if (!inspector)
  {
    inspector = new SegmentationInspector(segmentations, m_delegateFactory, m_context);

    connect(inspector, SIGNAL(inspectorClosed(SegmentationInspector*)),
            this,      SLOT(releaseInspectorResources(SegmentationInspector*)), Qt::DirectConnection);
    m_inspectors.insert(toKey(segmentations), inspector);
  }
  inspector->show();
  inspector->raise();
}

//------------------------------------------------------------------------
QModelIndexList SegmentationExplorer::Layout::indices(const QModelIndex& index, bool recursive)
{
  QModelIndexList res;

  Q_ASSERT(model() == index.model());

  for(int r = 0; r < model()->rowCount(index); r++)
  {
    auto child = index.child(r, 0);

    res << child;

    if (recursive)
    {
      res << indices(child, recursive);
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
void SegmentationExplorer::Layout::rowsAboutToBeRemoved(const QModelIndex parent, int start, int end)
{
  if (m_context.model()->segmentationRoot() == parent)
  {
    for(int row = start; row <= end; row++)
    {
      auto child = parent.child(row, 0);
      auto item = itemAdapter(child);
      if (isSegmentation(item))
      {
        auto segKey = toKey(segmentationPtr(item));

        for(auto key : m_inspectors.keys())
        {
          if (key.contains(segKey))
          {
            auto inspector = m_inspectors[key];
            if (key == segKey)
            {
              m_inspectors.remove(key);
              inspector->close();
            }
            else
            {
              QString newKey(key);

              m_inspectors.remove(key);
              m_inspectors.insert(newKey.remove(segKey), inspector);

              inspector->removeSegmentation(segmentationPtr(item));
            }
          }
        }
      }
    }
  }
}

//------------------------------------------------------------------------
QString SegmentationExplorer::Layout::toKey(SegmentationAdapterList segmentations)
{
  QString result;
  for(auto seg : segmentations)
    result += QString().number(reinterpret_cast<unsigned long long>(seg)) + QString("|");

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
  QString result = QString().number(reinterpret_cast<unsigned long long>(segmentation)) + QString("|");
  return result;
}

//------------------------------------------------------------------------
bool ESPINA::sortSegmentationLessThan(ItemAdapterPtr left, ItemAdapterPtr right)
{
  SegmentationAdapterPtr leftSeg  = segmentationPtr(left);
  SegmentationAdapterPtr rightSeg = segmentationPtr(right);

  if (leftSeg->category()->name() == rightSeg->category()->name())
  {
    QRegExp numExtractor("(\\d+)");
    numExtractor.setMinimal(false);

    auto stringLeft = leftSeg->data(Qt::DisplayRole).toString();
    auto stringRight = rightSeg->data(Qt::DisplayRole).toString();

    if ((numExtractor.indexIn(stringLeft) == -1) || (numExtractor.indexIn(stringRight) == -1))
      return stringLeft < stringRight;

    numExtractor.indexIn(stringLeft);
    auto numLeft = numExtractor.cap(1).toInt();

    numExtractor.indexIn(stringRight);
    auto numRight = numExtractor.cap(1).toInt();

    if (numLeft == numRight)
      return left ->data(Qt::ToolTipRole).toString() < right->data(Qt::ToolTipRole).toString();
    else
      return numLeft < numRight;
  }
  else
  {
    return leftSeg->category()->name() < rightSeg->category()->name();
  }

}
