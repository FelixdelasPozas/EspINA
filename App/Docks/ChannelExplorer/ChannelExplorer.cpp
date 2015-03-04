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
#include "ChannelExplorer.h"
#include "EspinaConfig.h"
#include <Menus/DefaultContextualMenu.h>
#include "Dialogs/ChannelInspector/ChannelInspector.h"
#include <GUI/Model/Utils/QueryAdapter.h>
#include <Core/Analysis/Channel.h>

// Qt
#include <QMessageBox>
#include <QUndoStack>
#include <QContextMenuEvent>
#include <ui_ChannelExplorer.h>

using namespace ESPINA;

//------------------------------------------------------------------------
class ChannelExplorer::CentralWidget
: public QWidget
, public Ui::ChannelExplorer
{
public:
  explicit CentralWidget(QWidget* parent = 0, Qt::WindowFlags f = 0)
  {
    setupUi(this);
    groupBox->setVisible(false);

    showInformation->setIcon(qApp->style()->standardIcon(QStyle::SP_MessageBoxInformation));
  }
};

//------------------------------------------------------------------------
ChannelExplorer::ChannelExplorer(ModelAdapterSPtr model,
                                 ViewManagerSPtr  viewManager,
                                 SchedulerSPtr    scheduler,
                                 QUndoStack      *undoStack,
                                 QWidget         *parent)
: DockWidget    {parent}
, m_model       {model}
, m_viewManager {viewManager}
, m_scheduler   {scheduler}
, m_undoStack   {undoStack}
, m_channelProxy{new ChannelProxy(model)}
, m_sort        {new QSortFilterProxyModel()}
, m_gui         {new CentralWidget()}
{
  setObjectName("ChannelExplorer");

  setWindowTitle(tr("Channel Explorer"));

  m_sort->setSourceModel(m_channelProxy.get());
  m_gui->view->setModel(m_sort.get());

  connect(m_channelProxy.get(), SIGNAL(channelsDragged(ChannelAdapterList, SampleAdapterPtr)),
          this,                 SLOT  (channelsDragged(ChannelAdapterList,SampleAdapterPtr)));


  connect(m_gui->showInformation, SIGNAL(clicked(bool)),
          this, SLOT(showInformation()));
  connect(m_gui->activeChannel, SIGNAL(clicked(bool)),
          this, SLOT(activateChannel()));
  connect(m_gui->alignLeft, SIGNAL(clicked(bool)),
          this, SLOT(alignLeft()));
  connect(m_gui->alignCenter, SIGNAL(clicked(bool)),
          this, SLOT(alignCenter()));
  connect(m_gui->alignRight, SIGNAL(clicked(bool)),
          this, SLOT(alignRight()));
  connect(m_gui->moveLeft, SIGNAL(clicked(bool)),
          this, SLOT(moveLelft()));
  connect(m_gui->moveRight, SIGNAL(clicked(bool)),
          this, SLOT(moveRight()));
  connect(m_gui->view, SIGNAL(clicked(QModelIndex)),
          this, SLOT(channelSelected()));
  connect(m_gui->view, SIGNAL(doubleClicked(QModelIndex)),
          this, SLOT(focusOnChannel()));
//   connect(m_gui->view, SIGNAL(itemStateChanged(QModelIndex)),
//           m_viewManager.get(), SLOT(updateChannelRepresentations()));
  connect(m_gui->xPos, SIGNAL(valueChanged(int)),
          this, SLOT(updateChannelPosition()));
  connect(m_gui->yPos, SIGNAL(valueChanged(int)),
          this, SLOT(updateChannelPosition()));
  connect(m_gui->zPos, SIGNAL(valueChanged(int)),
          this, SLOT(updateChannelPosition()));
  connect(m_gui->coordinateSelector, SIGNAL(currentIndexChanged(int)),
          this, SLOT(updateTooltips(int)));
  connect(m_gui->unloadChannel, SIGNAL(clicked(bool)),
          this, SLOT(unloadChannel()));

  updateTooltips(0);
  setWidget(m_gui);
}

//------------------------------------------------------------------------
ChannelExplorer::~ChannelExplorer()
{
//   qDebug() << "********************************************************";
//   qDebug() << "          Destroying Channel Explorer";
//   qDebug() << "********************************************************";
}

//------------------------------------------------------------------------
void ChannelExplorer::reset()
{
}

//------------------------------------------------------------------------
void ChannelExplorer::channelSelected()
{
//   QModelIndex currentIndex = m_gui->view->currentIndex();
//   if (!currentIndex.parent().isValid())
//     return;
//
//   QModelIndex index = m_sort->mapToSource(currentIndex);
//   ModelItemPtr currentItem = indexPtr(index);
//   if (ESPINA::CHANNEL == currentItem->type())
//   {
//     ChannelPtr channel = channelPtr(currentItem);
//     double pos[3];
//     channel->position(pos);
//     m_gui->xPos->blockSignals(true);
//     m_gui->yPos->blockSignals(true);
//     m_gui->zPos->blockSignals(true);
//
//     m_gui->xPos->setValue(pos[0]);
//     m_gui->yPos->setValue(pos[1]);
//     m_gui->zPos->setValue(pos[2]);
//
//     m_gui->xPos->blockSignals(false);
//     m_gui->yPos->blockSignals(false);
//     m_gui->zPos->blockSignals(false);
//   }
}

//------------------------------------------------------------------------
void ChannelExplorer::alignLeft()
{
//   QItemSelectionModel *selection = m_gui->view->selectionModel();
//   ChannelPtr lastChannel;
//   foreach (QModelIndex index, selection->selectedIndexes())
//   {
//     ModelItemPtr item = indexPtr(m_sort->mapToSource(index));
//     if (ESPINA::CHANNEL != item->type())
//       continue;
//
//     ChannelPtr channel = channelPtr(item);
//     if (lastChannel)
//     {
//       double lastPos[3], pos[3];
//       lastChannel->position(lastPos);
//       channel->position(pos);
//       int coord = m_gui->coordinateSelector->currentIndex();
//       pos[coord] = lastPos[coord];
//       channel->setPosition(pos);
//       channel->output()->update(); //FIXME: Check this is the right method (before it was volume()->update())
//     }
//     if (!lastChannel)
//       lastChannel = channel;
//   }
//   channelSelected();
}


//------------------------------------------------------------------------
void ChannelExplorer::alignCenter()
{
//   QItemSelectionModel *selection = m_gui->view->selectionModel();
//   ChannelPtr lastChannel;
//   double pos[3], bounds[6], centerMargin;
//   int coord = m_gui->coordinateSelector->currentIndex();
//
//   foreach (QModelIndex index, selection->selectedIndexes())
//   {
//     ModelItemPtr item = indexPtr(m_sort->mapToSource(index));
//     if (ESPINA::CHANNEL != item->type())
//       continue;
//
//     ChannelPtr channel = channelPtr(item);
//     if (lastChannel)
//     {
//       double pos[3], bounds[6];
//       channel->position(pos);
//       channel->volume()->bounds(bounds);
//       pos[coord] = centerMargin - (bounds[2*coord+1] - bounds[2*coord])/2.0;
//       channel->setPosition(pos);
//       channel->output()->update();
//     }
//
//     if (!lastChannel)
//     {
//       lastChannel = channel;
//       lastChannel->position(pos);
//       lastChannel->volume()->bounds(bounds);
//       centerMargin = pos[coord] + (bounds[2*coord+1] - bounds[2*coord])/2.0;
//     }
//   }
//   channelSelected();
}

//------------------------------------------------------------------------
void ChannelExplorer::alignRight()
{
//   QItemSelectionModel *selection = m_gui->view->selectionModel();
//   ChannelPtr lastChannel;
//   double pos[3], bounds[6], rightMargin;
//   int coord = m_gui->coordinateSelector->currentIndex();
//
//   foreach (QModelIndex index, selection->selectedIndexes())
//   {
//     ModelItemPtr item = indexPtr(m_sort->mapToSource(index));
//     if (ESPINA::CHANNEL != item->type())
//       continue;
//
//     ChannelPtr channel = channelPtr(item);
//     if (lastChannel)
//     {
//       double pos[3], bounds[6];
//       channel->position(pos);
//       channel->volume()->bounds(bounds);
//       pos[coord] = rightMargin - (bounds[2*coord+1] - bounds[2*coord]);
//       channel->setPosition(pos);
//       channel->output()->update();
//     }
//
//     if (!lastChannel)
//     {
//       lastChannel = channel;
//       lastChannel->position(pos);
//       lastChannel->volume()->bounds(bounds);
//       rightMargin = pos[coord] + (bounds[2*coord+1] - bounds[2*coord]);
//     }
//   }
//   channelSelected();
}

//------------------------------------------------------------------------
void ChannelExplorer::moveLelft()
{
//   QItemSelectionModel *selection = m_gui->view->selectionModel();
//   Channel *lastChannel = NULL;
//   double pos[3], leftMargin;
//   int coord = m_gui->coordinateSelector->currentIndex();
//
//   foreach (QModelIndex index, selection->selectedIndexes())
//   {
//     ModelItem *item = indexPtr(m_sort->mapToSource(index));
//     if (ESPINA::CHANNEL != item->type())
//       continue;
//
//     Channel *channel = dynamic_cast<Channel *>(item);
//     if (lastChannel)
//     {
//       double pos[3], bounds[6];
//       channel->position(pos);
//       channel->volume()->bounds(bounds);
//       pos[coord] = leftMargin - (bounds[2*coord+1] - bounds[2*coord]);
//       channel->setPosition(pos);
//       channel->notifyModification();
//     }
//
//     if (!lastChannel)
//     {
//       lastChannel = channel;
//       lastChannel->position(pos);
//       leftMargin = pos[coord];
//     }
//   }
//   channelSelected();
}

//------------------------------------------------------------------------
void ChannelExplorer::moveRight()
{
//   QItemSelectionModel *selection = m_gui->view->selectionModel();
//   Channel *lastChannel = NULL;
//   double pos[3], bounds[6], rightMargin;
//   int coord = m_gui->coordinateSelector->currentIndex();
//
//   foreach (QModelIndex index, selection->selectedIndexes())
//   {
//     ModelItem *item = indexPtr(m_sort->mapToSource(index));
//     if (ESPINA::CHANNEL != item->type())
//       continue;
//
//     Channel *channel = dynamic_cast<Channel *>(item);
//     if (lastChannel)
//     {
//       double pos[3], bounds[6];
//       channel->position(pos);
//       channel->volume()->bounds(bounds);
//       pos[coord] = rightMargin;
//       channel->setPosition(pos);
//       channel->notifyModification();
//     }
//
//     if (!lastChannel)
//     {
//       lastChannel = channel;
//       lastChannel->position(pos);
//       lastChannel->volume()->bounds(bounds);
//       rightMargin = pos[coord] + (bounds[2*coord+1] - bounds[2*coord]);
//     }
//   }
//   channelSelected();
}

//------------------------------------------------------------------------
void ChannelExplorer::updateChannelPosition()
{
//   QModelIndex currentIndex = m_gui->view->currentIndex();
//   if (!currentIndex.parent().isValid())
//     return;
//
//   QModelIndex index = m_sort->mapToSource(currentIndex);
//   ModelItemPtr currentItem = indexPtr(index);
//   if (ESPINA::CHANNEL == currentItem->type())
//   {
//     ChannelPtr channel = channelPtr(currentItem);
//     double pos[3] = {
//       static_cast<double>(m_gui->xPos->value()),
//       static_cast<double>(m_gui->yPos->value()),
//       static_cast<double>(m_gui->zPos->value())
//     };
//
//     channel->setPosition(pos);
//     channel->output()->update();
//   }
}

//------------------------------------------------------------------------
void ChannelExplorer::updateTooltips(int index)
{
//  if (0 == index)
//  {
//    m_gui->alignLeft->setToolTip(tr("Align Left Margins"));
//    m_gui->alignCenter->setToolTip(tr("Center Margins in plane X"));
//    m_gui->alignRight->setToolTip(tr("Align Right Margins"));
//    m_gui->moveLeft->setToolTip(tr("Move next to Left Margin"));
//    m_gui->moveRight->setToolTip(tr("Move next to Right Margin"));
//  } else if (1 == index)
//  {
//    m_gui->alignLeft->setToolTip(tr("Align Top Margins"));
//    m_gui->alignCenter->setToolTip(tr("Center Margins in plane Y"));
//    m_gui->alignRight->setToolTip(tr("Align Bottom Margins"));
//    m_gui->moveLeft->setToolTip(tr("Move next to Top Margin"));
//    m_gui->moveRight->setToolTip(tr("Move next to Bottom Margin"));
//  } else if (2 == index)
//  {
//    m_gui->alignLeft->setToolTip(tr("Align Lower Margins"));
//    m_gui->alignCenter->setToolTip(tr("Center Margins in plane Z"));
//    m_gui->alignRight->setToolTip(tr("Align Upper Margins"));
//    m_gui->moveLeft->setToolTip(tr("Move next to Lower Margin"));
//    m_gui->moveRight->setToolTip(tr("Move next to Upper Margin"));
//  }else
//    Q_ASSERT(false);
}

//------------------------------------------------------------------------
void ChannelExplorer::unloadChannel()
{
//   QModelIndex index = m_sort->mapToSource(m_gui->view->currentIndex());
//   if (!index.isValid())
//     return;
//
//   ModelItemPtr item = indexPtr(index);
//   if (ESPINA::CHANNEL != item->type())
//     return;
//
//   ChannelPtr channel = channelPtr(item);
//   ModelItemSList relItems = channel->relatedItems(ESPINA::RELATION_OUT);
//
//   if (!relItems.empty())
//   {
//     QString msgText;
//     if (relItems.size() > 1)
//     {
//       QString number;
//       number.setNum(relItems.size());
//       msgText = QString("That channel cannot be unloaded because there are ") + number + QString(" segmentations that depend on it.");
//     }
//
//     else
//       msgText = QString("That channel cannot be unloaded because there is a segmentation that depends on it.");
//     QMessageBox msgBox;
//     msgBox.setIcon(QMessageBox::Information);
//     msgBox.setText(msgText);
//     msgBox.setStandardButtons(QMessageBox::Ok);
//     msgBox.exec();
//     return;
//   }
//   else
//   {
//     SampleSPtr sample = channel->sample();
//     m_undoStack->beginMacro("Unload Channel");
//     {
//       m_undoStack->push(new UnloadChannelCommand(channel, m_model));
//       if (sample->channels().isEmpty())
//       {
//         m_undoStack->push(new UnloadSampleCommand(sample, m_model));
//       }
//     }
//     m_undoStack->endMacro();
//   }
}

//------------------------------------------------------------------------
void ChannelExplorer::focusOnChannel()
{
//   QModelIndex currentIndex = m_gui->view->currentIndex();
//   if (!currentIndex.parent().isValid())
//     return;
//
//   QModelIndex index = m_sort->mapToSource(currentIndex);
//   ModelItemPtr currentItem = indexPtr(index);
//   if (ESPINA::CHANNEL == currentItem->type())
//   {
//     ChannelPtr channel = channelPtr(currentItem);
//     Nm bounds[6];
//     channel->volume()->bounds(bounds);
//     //TODO 2012-10-04: Use setSelection instead of setCameraFocus
// //     double pos[3] = { (bounds[1]-bounds[0])/2, (bounds[3]-bounds[2])/2, (bounds[5]-bounds[4])/2 };
// //     EspinaView *view = EspinaCore::instance()->viewManger()->currentView();
// //     view->setCameraFocus(pos);
// //     view->forceRender();
//   }
}

//------------------------------------------------------------------------
void ChannelExplorer::showInformation()
{
   for(auto index: m_gui->view->selectionModel()->selectedIndexes())
   {
     QModelIndex currentIndex = m_sort->mapToSource(index);
     ItemAdapterPtr currentItem = itemAdapter(currentIndex);
     Q_ASSERT(currentItem);

     if (ItemAdapter::Type::CHANNEL == currentItem->type())
     {
       ChannelAdapterPtr channel = channelPtr(currentItem);
       ChannelInspector *inspector = m_informationDialogs.value(channel, nullptr);

       if (!inspector)
       {
         inspector = new ChannelInspector(channel, m_model, m_scheduler);
         m_informationDialogs.insert(channel, inspector);
         connect(inspector, SIGNAL(destroyed(QObject *)), this, SLOT(dialogClosed(QObject *)));
         connect(inspector, SIGNAL(spacingUpdated()), this, SLOT(inspectorChangedSpacing()));
       }
       inspector->show();
       inspector->raise();
     }
   }
}

//------------------------------------------------------------------------
void ChannelExplorer::activateChannel()
{
  QModelIndex currentIndex = m_gui->view->currentIndex();
  if (!currentIndex.parent().isValid())
    return;

  auto index = m_sort->mapToSource(currentIndex);
  auto currentItem = itemAdapter(index);

  if (ItemAdapter::Type::CHANNEL == currentItem->type())
  {
    auto currentChannel = channelPtr(currentItem);
    m_viewManager->setActiveChannel(currentChannel);
  }
}

//------------------------------------------------------------------------
void ChannelExplorer::dialogClosed(QObject *object)
{
  auto dialog = static_cast<ChannelInspector *>(object);
  auto channel = m_informationDialogs.key(dialog);
  Q_ASSERT(channel);
  channel->output()->update();

  ChannelAdapterList channels;
  channels << channel;

  //TODO: update representations m_viewManager->updateChannelRepresentations(channels);

  SegmentationAdapterList segmentations;
  for (auto segmentation : QueryAdapter::segmentationsOnChannelSample(channel))
  {
    segmentations << segmentation.get();
  }

  //TODO: update representations
  //m_viewManager->updateSegmentationRepresentations(segmentations);
  //m_viewManager->updateViews();

  m_informationDialogs.remove(channel);
}

//------------------------------------------------------------------------
void ChannelExplorer::inspectorChangedSpacing()
{
  m_viewManager->resetViewCameras();
}

//------------------------------------------------------------------------
void ChannelExplorer::channelsDragged(ChannelAdapterList channels, SampleAdapterPtr sample)
{
  auto smartSample = m_model->smartPointer(sample);

  for (auto channel : channels)
  {
    auto prevSample   = QueryAdapter::sample(channel);
    auto smartChannel = m_model->smartPointer(channel);

    m_model->deleteRelation(prevSample, smartChannel, Channel::STAIN_LINK);
    m_model->addRelation   (smartSample, smartChannel, Channel::STAIN_LINK);
  }
}

//------------------------------------------------------------------------
void ESPINA::ChannelExplorer::contextMenuEvent(QContextMenuEvent *e)
{
  ChannelAdapterList channels;

  for(auto index : m_gui->view->selectionModel()->selectedIndexes())
  {
    auto item = itemAdapter(m_sort->mapToSource(index));

    if (isChannel(item))
    {
      channels << channelPtr(item);
    }
  }

  if(channels.empty() || channels.size() != 1)
    return;

  QMenu contextMenu;

  auto setActive = contextMenu.addAction(tr("Set as the active channel"));
  setActive->setCheckable(true);
  setActive->setChecked(channels.first() == m_viewManager->activeChannel());
  connect(setActive, SIGNAL(triggered(bool)),
          this,      SLOT(activateChannel()));

  contextMenu.addSeparator();

  auto properties = contextMenu.addAction(tr("Channel properties..."));
  connect(properties, SIGNAL(triggered(bool)),
          this,       SLOT(showInformation()));

  contextMenu.exec(e->globalPos());
}
