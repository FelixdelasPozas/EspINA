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

#ifdef DEBUG
#include "common/model/ModelTest.h"
#include <model/Channel.h>
#include <qcolordialog.h>
#endif


//------------------------------------------------------------------------
class ChannelExplorer::CentralWidget
: public QWidget
, public Ui::ChannelExplorer
{
public:
  explicit CentralWidget(QWidget* parent = 0, Qt::WindowFlags f = 0)
  {
    setupUi(this);
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
  connect(m_gui->xPos, SIGNAL(valueChanged(int)),
	  this, SLOT(updateChannelPosition()));
  connect(m_gui->yPos, SIGNAL(valueChanged(int)),
	  this, SLOT(updateChannelPosition()));
  connect(m_gui->zPos, SIGNAL(valueChanged(int)),
	  this, SLOT(updateChannelPosition()));

  setWidget(m_gui);
}

//------------------------------------------------------------------------
ChannelExplorer::~ChannelExplorer()
{

}

//------------------------------------------------------------------------
void ChannelExplorer::alignLeft()
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
    int pos[3];
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
void ChannelExplorer::alignCenter()
{

}

//------------------------------------------------------------------------
void ChannelExplorer::alignRight()
{

}

//------------------------------------------------------------------------
void ChannelExplorer::moveLelft()
{

}

//------------------------------------------------------------------------
void ChannelExplorer::moveRight()
{

}

//------------------------------------------------------------------------
void ChannelExplorer::changeChannelColor()
{
  QColorDialog colorSelector;
  if( colorSelector.exec() == QDialog::Accepted)
  {
//     QModelIndex index = m_sort->mapToSource(m_gui->treeView->currentIndex());
//     m_baseModel->setData(index,
// 			 colorSelector.selectedColor(),
// 			 Qt::DecorationRole);
  }
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
    int pos[3] = {
      m_gui->xPos->value(),
      m_gui->yPos->value(),
      m_gui->zPos->value()
    };

    channel->setPosition(pos);
  }
}

