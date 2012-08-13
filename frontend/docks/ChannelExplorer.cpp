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
#include <model/ModelItem.h>
#include <model/EspinaModel.h>
#include <common/EspinaCore.h>
#include <QMessageBox>

#include <ui_ChannelExplorer.h>

#include "EspinaConfig.h"

#ifdef TEST_ESPINA_MODELS
  #include "common/model/ModelTest.h"
#endif

#include <model/Channel.h>
#include <gui/HueSelector.h>

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
  }
};

//------------------------------------------------------------------------
ChannelExplorer::ChannelExplorer(QSharedPointer< EspinaModel > model,
				 QWidget* parent)
: EspinaDockWidget(parent)
, m_gui(new CentralWidget())
, m_model(model)
, m_channelProxy(new ChannelProxy())
, m_sort(new QSortFilterProxyModel())
{
  setWindowTitle(tr("Channel Explorer"));
  setObjectName("ChannelExplorer");

  m_channelProxy->setSourceModel(m_model.data());
  m_sort->setSourceModel(m_channelProxy.data());
  m_gui->view->setModel(m_sort.data());

  connect(m_gui->channelColor, SIGNAL(clicked(bool)), this, SLOT(changeChannelColor()));
  connect(m_gui->alignLeft, SIGNAL(clicked(bool)), this, SLOT(alignLeft()));
  connect(m_gui->alignCenter, SIGNAL(clicked(bool)), this, SLOT(alignCenter()));
  connect(m_gui->alignRight, SIGNAL(clicked(bool)), this, SLOT(alignRight()));
  connect(m_gui->moveLeft, SIGNAL(clicked(bool)), this, SLOT(moveLelft()));
  connect(m_gui->moveRight, SIGNAL(clicked(bool)), this, SLOT(moveRight()));
  connect(m_gui->view, SIGNAL(clicked(QModelIndex)), this, SLOT(channelSelected()));
  connect(m_gui->xPos, SIGNAL(valueChanged(int)), this, SLOT(updateChannelPosition()));
  connect(m_gui->yPos, SIGNAL(valueChanged(int)), this, SLOT(updateChannelPosition()));
  connect(m_gui->zPos, SIGNAL(valueChanged(int)), this, SLOT(updateChannelPosition()));
  connect(m_gui->coordinateSelector, SIGNAL(currentIndexChanged(int)), this, SLOT(updateTooltips(int)));
  connect(m_gui->unloadChannel, SIGNAL(clicked(bool)), this, SLOT(unloadChannel()));

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
      channel->bounds(bounds);
      pos[coord] = centerMargin - (bounds[2*coord+1] - bounds[2*coord])/2.0;
      channel->setPosition(pos);
      channel->notifyModification();
    }

    if (!lastChannel)
    {
      lastChannel = channel;
      lastChannel->position(pos);
      lastChannel->bounds(bounds);
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
      channel->bounds(bounds);
      pos[coord] = rightMargin - (bounds[2*coord+1] - bounds[2*coord]);
      channel->setPosition(pos);
      channel->notifyModification();
    }

    if (!lastChannel)
    {
      lastChannel = channel;
      lastChannel->position(pos);
      lastChannel->bounds(bounds);
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
      channel->bounds(bounds);
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
      channel->bounds(bounds);
      pos[coord] = rightMargin;
      channel->setPosition(pos);
      channel->notifyModification();
    }

    if (!lastChannel)
    {
      lastChannel = channel;
      lastChannel->position(pos);
      lastChannel->bounds(bounds);
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
      m_gui->xPos->value(),
      m_gui->yPos->value(),
      m_gui->zPos->value()
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
  QSharedPointer<EspinaModel> model = EspinaCore::instance()->model();
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
          model->removeRelation((*it), item, Channel::STAINLINK);
          model->removeSample(reinterpret_cast<Sample *>(*it));
          delete (*it);
        }
      }
      else
      {
        model->removeRelation((*it), item, Channel::VOLUMELINK);
        model->removeFilter(reinterpret_cast<Filter *>(*it));
        delete (*it);
      }
      it++;
    }
    model->removeChannel(channel);
  }
}
