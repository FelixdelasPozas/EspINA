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

#include "ChannelExplorer.h"
#include <ui_ChannelExplorer.h>

#include "EspinaConfig.h"
#include "Docks/ChannelInspector/ChannelInspector.h"

// EspINA
#include <GUI/QtWidget/HueSelector.h>
#include <GUI/ViewManager.h>
#include <Core/Model/Channel.h>
#include <Core/Model/EspinaModel.h>
#include <Core/Model/ModelItem.h>

#ifdef TEST_ESPINA_MODELS
  #include <Core/Model/ModelTest.h>
#endif

// Qt
#include <QMessageBox>

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

    showInformation->setIcon(
      qApp->style()->standardIcon(QStyle::SP_MessageBoxInformation));
  }
};

//------------------------------------------------------------------------
ChannelExplorer::ChannelExplorer(EspinaModel *model,
                                 ViewManager *vm,
                                 QWidget     *parent)
: QDockWidget(parent)
, m_gui(new CentralWidget())
, m_model(model)
, m_viewManager(vm)
, m_channelProxy(new ChannelProxy(vm))
, m_sort(new QSortFilterProxyModel())
{
  setWindowTitle(tr("Channel Explorer"));
  setObjectName("ChannelExplorer");

  m_channelProxy->setSourceModel(m_model);
  m_sort->setSourceModel(m_channelProxy.data());
  m_gui->view->setModel(m_sort.data());

  connect(m_gui->showInformation, SIGNAL(clicked(bool)),
          this, SLOT(showInformation()));
  connect(m_gui->activeChannel, SIGNAL(clicked(bool)),
          this, SLOT(activateChannel()));
  connect(m_gui->channelColor, SIGNAL(clicked(bool)),
          this, SLOT(changeChannelColor()));
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
  connect(m_gui->view, SIGNAL(itemStateChanged(QModelIndex)),
          m_viewManager, SLOT(updateViews()));
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

}

//------------------------------------------------------------------------
void ChannelExplorer::channelSelected()
{
  QModelIndex currentIndex = m_gui->view->currentIndex();
  if (!currentIndex.parent().isValid())
    return;

  QModelIndex index = m_sort->mapToSource(currentIndex);
  ModelItem *currentItem = indexPtr(index);
  if (ModelItem::CHANNEL == currentItem->type())
  {
    Channel *channel = dynamic_cast<Channel *>(currentItem);
    double pos[3];
    channel->position(pos);
    m_gui->xPos->blockSignals(true);
    m_gui->yPos->blockSignals(true);
    m_gui->zPos->blockSignals(true);

    m_gui->xPos->setValue(pos[0]);
    m_gui->yPos->setValue(pos[1]);
    m_gui->zPos->setValue(pos[2]);

    m_gui->xPos->blockSignals(false);
    m_gui->yPos->blockSignals(false);
    m_gui->zPos->blockSignals(false);
  }
}

//------------------------------------------------------------------------
void ChannelExplorer::alignLeft()
{
  QItemSelectionModel *selection = m_gui->view->selectionModel();
  Channel *lastChannel = NULL;
  foreach (QModelIndex index, selection->selectedIndexes())
  {
    ModelItem *item = indexPtr(m_sort->mapToSource(index));
    if (ModelItem::CHANNEL != item->type())
      continue;

    Channel *channel = dynamic_cast<Channel *>(item);
    if (lastChannel)
    {
      double lastPos[3], pos[3];
      lastChannel->position(lastPos);
      channel->position(pos);
      int coord = m_gui->coordinateSelector->currentIndex();
      pos[coord] = lastPos[coord];
      channel->setPosition(pos);
      channel->notifyModification();
    }
    if (!lastChannel)
      lastChannel = channel;
  }
  channelSelected();
}


//------------------------------------------------------------------------
void ChannelExplorer::alignCenter()
{
  QItemSelectionModel *selection = m_gui->view->selectionModel();
  Channel *lastChannel = NULL;
  double pos[3], bounds[6], centerMargin;
  int coord = m_gui->coordinateSelector->currentIndex();

  foreach (QModelIndex index, selection->selectedIndexes())
  {
    ModelItem *item = indexPtr(m_sort->mapToSource(index));
    if (ModelItem::CHANNEL != item->type())
      continue;

    Channel *channel = dynamic_cast<Channel *>(item);
    if (lastChannel)
    {
      double pos[3], bounds[6];
      channel->position(pos);
      channel->volume()->bounds(bounds);
      pos[coord] = centerMargin - (bounds[2*coord+1] - bounds[2*coord])/2.0;
      channel->setPosition(pos);
      channel->notifyModification();
    }

    if (!lastChannel)
    {
      lastChannel = channel;
      lastChannel->position(pos);
      lastChannel->volume()->bounds(bounds);
      centerMargin = pos[coord] + (bounds[2*coord+1] - bounds[2*coord])/2.0;
    }
  }
  channelSelected();
}

//------------------------------------------------------------------------
void ChannelExplorer::alignRight()
{
  QItemSelectionModel *selection = m_gui->view->selectionModel();
  Channel *lastChannel = NULL;
  double pos[3], bounds[6], rightMargin;
  int coord = m_gui->coordinateSelector->currentIndex();

  foreach (QModelIndex index, selection->selectedIndexes())
  {
    ModelItem *item = indexPtr(m_sort->mapToSource(index));
    if (ModelItem::CHANNEL != item->type())
      continue;

    Channel *channel = dynamic_cast<Channel *>(item);
    if (lastChannel)
    {
      double pos[3], bounds[6];
      channel->position(pos);
      channel->volume()->bounds(bounds);
      pos[coord] = rightMargin - (bounds[2*coord+1] - bounds[2*coord]);
      channel->setPosition(pos);
      channel->notifyModification();
    }

    if (!lastChannel)
    {
      lastChannel = channel;
      lastChannel->position(pos);
      lastChannel->volume()->bounds(bounds);
      rightMargin = pos[coord] + (bounds[2*coord+1] - bounds[2*coord]);
    }
  }
  channelSelected();
}

//------------------------------------------------------------------------
void ChannelExplorer::moveLelft()
{
  QItemSelectionModel *selection = m_gui->view->selectionModel();
  Channel *lastChannel = NULL;
  double pos[3], leftMargin;
  int coord = m_gui->coordinateSelector->currentIndex();

  foreach (QModelIndex index, selection->selectedIndexes())
  {
    ModelItem *item = indexPtr(m_sort->mapToSource(index));
    if (ModelItem::CHANNEL != item->type())
      continue;

    Channel *channel = dynamic_cast<Channel *>(item);
    if (lastChannel)
    {
      double pos[3], bounds[6];
      channel->position(pos);
      channel->volume()->bounds(bounds);
      pos[coord] = leftMargin - (bounds[2*coord+1] - bounds[2*coord]);
      channel->setPosition(pos);
      channel->notifyModification();
    }

    if (!lastChannel)
    {
      lastChannel = channel;
      lastChannel->position(pos);
      leftMargin = pos[coord];
    }
  }
  channelSelected();
}

//------------------------------------------------------------------------
void ChannelExplorer::moveRight()
{
  QItemSelectionModel *selection = m_gui->view->selectionModel();
  Channel *lastChannel = NULL;
  double pos[3], bounds[6], rightMargin;
  int coord = m_gui->coordinateSelector->currentIndex();

  foreach (QModelIndex index, selection->selectedIndexes())
  {
    ModelItem *item = indexPtr(m_sort->mapToSource(index));
    if (ModelItem::CHANNEL != item->type())
      continue;

    Channel *channel = dynamic_cast<Channel *>(item);
    if (lastChannel)
    {
      double pos[3], bounds[6];
      channel->position(pos);
      channel->volume()->bounds(bounds);
      pos[coord] = rightMargin;
      channel->setPosition(pos);
      channel->notifyModification();
    }

    if (!lastChannel)
    {
      lastChannel = channel;
      lastChannel->position(pos);
      lastChannel->volume()->bounds(bounds);
      rightMargin = pos[coord] + (bounds[2*coord+1] - bounds[2*coord]);
    }
  }
  channelSelected();
}

//------------------------------------------------------------------------
void ChannelExplorer::changeChannelColor()
{
  QModelIndex index = m_sort->mapToSource(m_gui->view->currentIndex());
  if (!index.isValid())
    return;

  ModelItem *item = indexPtr(index);
  if (ModelItem::CHANNEL != item->type())
    return;

  Channel *channel = dynamic_cast<Channel *>(item);

  HueSelector *hueSelector = new HueSelector(channel->color(), this);
  hueSelector->exec();

  if(hueSelector->ModifiedData())
  {
    double value = (hueSelector->GetHueValue() == -1) ? -1 : (hueSelector->GetHueValue() / 359.);
    channel->setColor(value);
    channel->notifyModification();
  }
  delete hueSelector;
}

//------------------------------------------------------------------------
void ChannelExplorer::updateChannelPosition()
{
  QModelIndex currentIndex = m_gui->view->currentIndex();
  if (!currentIndex.parent().isValid())
    return;

  QModelIndex index = m_sort->mapToSource(currentIndex);
  ModelItem *currentItem = indexPtr(index);
  if (ModelItem::CHANNEL == currentItem->type())
  {
    Channel *channel = dynamic_cast<Channel *>(currentItem);
    double pos[3] = {
      static_cast<double>(m_gui->xPos->value()),
      static_cast<double>(m_gui->yPos->value()),
      static_cast<double>(m_gui->zPos->value())
    };

    channel->setPosition(pos);
    channel->notifyModification();
  }
}

//------------------------------------------------------------------------
void ChannelExplorer::updateTooltips(int index)
{
  if (0 == index)
  {
    m_gui->alignLeft->setToolTip(tr("Align Left Margins"));
    m_gui->alignCenter->setToolTip(tr("Center Margins in plane X"));
    m_gui->alignRight->setToolTip(tr("Align Right Margins"));
    m_gui->moveLeft->setToolTip(tr("Move next to Left Margin"));
    m_gui->moveRight->setToolTip(tr("Move next to Right Margin"));
  } else if (1 == index)
  {
    m_gui->alignLeft->setToolTip(tr("Align Top Margins"));
    m_gui->alignCenter->setToolTip(tr("Center Margins in plane Y"));
    m_gui->alignRight->setToolTip(tr("Align Bottom Margins"));
    m_gui->moveLeft->setToolTip(tr("Move next to Top Margin"));
    m_gui->moveRight->setToolTip(tr("Move next to Bottom Margin"));
  } else if (2 == index)
  {
    m_gui->alignLeft->setToolTip(tr("Align Lower Margins"));
    m_gui->alignCenter->setToolTip(tr("Center Margins in plane Z"));
    m_gui->alignRight->setToolTip(tr("Align Upper Margins"));
    m_gui->moveLeft->setToolTip(tr("Move next to Lower Margin"));
    m_gui->moveRight->setToolTip(tr("Move next to Upper Margin"));
  }else
    Q_ASSERT(false);
}

//------------------------------------------------------------------------
void ChannelExplorer::unloadChannel()
{
  QModelIndex index = m_sort->mapToSource(m_gui->view->currentIndex());
  if (!index.isValid())
    return;

  ModelItem *item = indexPtr(index);
  if (ModelItem::CHANNEL != item->type())
    return;

  Channel *channel = dynamic_cast<Channel *>(item);
  ModelItem::Vector relItems = channel->relatedItems(ModelItem::OUT);

  if (!relItems.empty())
  {
    QString msgText;
    if (relItems.size() > 1)
    {
      QString number;
      number.setNum(relItems.size());
      msgText = QString("That channel cannot be deleted because there are ") + number + QString(" segmentations that depend on it.");
    }

    else
      msgText = QString("That channel cannot be deleted because there is a segmentation that depends on it.");
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Information);
    msgBox.setText(msgText);
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.exec();
    return;
  }
  else
  {
    relItems = channel->relatedItems(ModelItem::IN);
    ModelItem::Vector::Iterator it = relItems.begin();
    Q_ASSERT(relItems.size() == 2);
    while (it != relItems.end())
    {
      if ((*it)->type() == ModelItem::SAMPLE)
      {
        ModelItem::Vector relatedItems = (*it)->relatedItems(ModelItem::OUT);
        if (relatedItems.size() == 1)
        {
          m_model->removeRelation((*it), item, Channel::STAINLINK);
          m_model->removeSample(reinterpret_cast<Sample *>(*it));
          delete (*it);
        }
      }
      else
      {
        m_model->removeRelation((*it), item, Channel::VOLUMELINK);
        m_model->removeFilter(reinterpret_cast<Filter *>(*it));
        delete (*it);
      }
      it++;
    }

    m_model->removeChannel(channel);

    if (m_viewManager->activeChannel() == channel)
      m_viewManager->setActiveChannel(NULL);
  }
}

//------------------------------------------------------------------------
void ChannelExplorer::focusOnChannel()
{
  QModelIndex currentIndex = m_gui->view->currentIndex();
  if (!currentIndex.parent().isValid())
    return;

  QModelIndex index = m_sort->mapToSource(currentIndex);
  ModelItem *currentItem = indexPtr(index);
  if (ModelItem::CHANNEL == currentItem->type())
  {
    Channel *channel = dynamic_cast<Channel *>(currentItem);
    Nm bounds[6];
    channel->volume()->bounds(bounds);
    //TODO 2012-10-04: Use setSelection instead of setCameraFocus
//     double pos[3] = { (bounds[1]-bounds[0])/2, (bounds[3]-bounds[2])/2, (bounds[5]-bounds[4])/2 };
//     EspinaView *view = EspinaCore::instance()->viewManger()->currentView();
//     view->setCameraFocus(pos);
//     view->forceRender();
  }
}

//------------------------------------------------------------------------
void ChannelExplorer::showInformation()
{
  foreach(QModelIndex index, m_gui->view->selectionModel()->selectedIndexes())
  {
    QModelIndex currentIndex = m_sort->mapToSource(index);
    ModelItem *currentItem = indexPtr(currentIndex);
    Q_ASSERT(currentItem);

    if (ModelItem::CHANNEL == currentItem->type())
    {
      Channel *channel = dynamic_cast<Channel *>(currentItem);
      ChannelInspector *inspector = new ChannelInspector(channel, m_viewManager);
      inspector->exec();
    }
  }
}

//------------------------------------------------------------------------
void ChannelExplorer::activateChannel()
{
    QModelIndex currentIndex = m_gui->view->currentIndex();
  if (!currentIndex.parent().isValid())
    return;

  QModelIndex index = m_sort->mapToSource(currentIndex);
  ModelItem *currentItem = indexPtr(index);
  if (ModelItem::CHANNEL == currentItem->type())
  {
    Channel *currentChannel = dynamic_cast<Channel *>(currentItem);
    m_viewManager->setActiveChannel(currentChannel);
  }
}