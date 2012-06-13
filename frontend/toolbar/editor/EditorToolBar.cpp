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


#include "EditorToolBar.h"

#include <selection/SelectionManager.h>
#include <selection/SelectableItem.h>
#include <selection/PixelSelector.h>

#include <QAction>
#include <EspinaCore.h>
#include <model/Channel.h>
#include <editor/ImageLogicCommand.h>
#include <editor/PencilSelector.h>
#include <editor/FreeFormSource.h>

//----------------------------------------------------------------------------
EditorToolBar::EditorToolBar(QWidget* parent)
: QToolBar(parent)
, m_draw(addAction(tr("Draw segmentations")))
, m_addition(addAction(tr("Combine Selected Segmentations")))
, m_substraction(addAction(tr("Substract Selected Segmentations")))
, m_pencilSelector(new PencilSelector())
, m_currentSource(NULL)
, m_currentSeg(NULL)
{
//   setWindowTitle("Editor Tool Bar");
  setObjectName("EditorToolBar");

  m_draw->setIcon(QIcon(":/espina/pencil.png"));
  m_draw->setCheckable(true);
  connect(m_draw, SIGNAL(triggered(bool)),
	 this, SLOT(startDrawing(bool)));
  m_addition->setIcon(QIcon(":/espina/add.svg"));
  connect(m_addition, SIGNAL(triggered(bool)),
	 this, SLOT(combineSegmentations()));
  m_substraction->setIcon(QIcon(":/espina/remove.svg"));
  connect(m_substraction, SIGNAL(triggered(bool)),
	 this, SLOT(substractSegmentations()));

  m_pencilSelector->setSelectable(SelectionHandler::EspINA_Channel);
  connect(m_pencilSelector, SIGNAL(selectionChanged(SelectionHandler::MultiSelection)),
	 this, SLOT(drawSegmentation(SelectionHandler::MultiSelection)));
  connect(m_pencilSelector, SIGNAL(selectionAborted()),
	 this, SLOT(stopDrawing()));
  connect(m_pencilSelector, SIGNAL(stateChanged(PencilSelector::State)),
	 this, SLOT(stateChanged(PencilSelector::State)));
}

//----------------------------------------------------------------------------
void EditorToolBar::startDrawing(bool draw)
{
  if (draw)
    SelectionManager::instance()->setSelectionHandler(m_pencilSelector);
  else
  {
    SelectionManager::instance()->setSelectionHandler(NULL);
    m_currentSource = NULL;
    m_currentSeg = NULL;
  }
}

//----------------------------------------------------------------------------
void EditorToolBar::drawSegmentation(SelectionHandler::MultiSelection msel)
{
  if (msel.size() == 0)
    return;

  SelectionHandler::VtkRegion region = msel.first().first;
  if (region.size() != 2)
    return;

  QVector3D center = region[0];
  QVector3D p = region[1];
  int extent[6];
  extent[0] = center.x() - 5*m_pencilSelector->radius();
  extent[1] = center.x() + 5*m_pencilSelector->radius();
  extent[2] = center.y() - 5*m_pencilSelector->radius();
  extent[3] = center.y() + 5*m_pencilSelector->radius();
  extent[4] = center.z() - 5*m_pencilSelector->radius();
  extent[5] = center.z() + 5*m_pencilSelector->radius();

  double spacing[3];
  SelectableItem *selectedItem = msel.first().second;
  if (ModelItem::CHANNEL == selectedItem->type())
  {
    int channelExtent[6];
    Channel *channel = dynamic_cast<Channel *>(selectedItem);
    channel->extent(channelExtent);
    for(int i=0; i<3; i++)
    {
      int min = 2*i, max = 2*i+1;
      extent[min] = std::max(extent[min], channelExtent[min]);
      extent[max] = std::min(extent[max], channelExtent[max]);
    }
    channel->spacing(spacing);
//     center.setX(center.x()*spacing[0]);
//     center.setY(center.y()*spacing[1]);
//     center.setZ(center.z()*spacing[2]);
  }

  if (!m_currentSource)
  {
    m_currentSource = new FreeFormSource(spacing);
  }

  int radius = abs(center.x() - p.x());

  if (m_pencilSelector->state() == PencilSelector::DRAWING)
    m_currentSource->draw(center, radius);
  else if (m_pencilSelector->state() == PencilSelector::ERASING)
    m_currentSource->erase(center, radius);
  else
    Q_ASSERT(false);

  if (!m_currentSeg)
  {
    m_currentSeg = m_currentSource->product(0);
    m_currentSeg->setTaxonomy(EspinaCore::instance()->activeTaxonomy());
    EspinaCore::instance()->model()->addSegmentation(m_currentSeg);
  }
  m_currentSeg->notifyModification(true);
}

//----------------------------------------------------------------------------
void EditorToolBar::stopDrawing()
{
  m_draw->blockSignals(true);
  m_draw->setChecked(false);
  m_pencilSelector->changeState(PencilSelector::DRAWING);
  m_draw->blockSignals(false);
}

//----------------------------------------------------------------------------
void EditorToolBar::stateChanged(PencilSelector::State state)
{
  switch (state)
  {
    case PencilSelector::DRAWING:
      m_draw->setIcon(QIcon(":/espina/pencil.png"));
      break;
    case PencilSelector::ERASING:
      m_draw->setIcon(QIcon(":/espina/eraser.png"));
      break;
  };
}

//----------------------------------------------------------------------------
void EditorToolBar::combineSegmentations()
{
  QSharedPointer<EspinaModel> model = EspinaCore::instance()->model();

  QList<Segmentation *> input;
  foreach(Segmentation *seg, model->segmentations())
  {
    if (seg->isSelected())
      input << seg;
  }

  if (!input.isEmpty())
  {
    QSharedPointer<QUndoStack> undo(EspinaCore::instance()->undoStack());
    undo->push(new ImageLogicCommand(input, ImageLogicFilter::ADDITION));
  }
}

//----------------------------------------------------------------------------
void EditorToolBar::substractSegmentations()
{
  QSharedPointer<EspinaModel> model = EspinaCore::instance()->model();

  QList<Segmentation *> input;
  foreach(Segmentation *seg, model->segmentations())
  {
    if (seg->isSelected())
      input << seg;
  }

  if (!input.isEmpty())
  {
    QSharedPointer<QUndoStack> undo(EspinaCore::instance()->undoStack());
    undo->push(new ImageLogicCommand(input, ImageLogicFilter::SUBSTRACTION));
  }
}
