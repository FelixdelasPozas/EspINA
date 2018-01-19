/*

 Copyright (C) 2017 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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
#include <Core/Utils/Bounds.h>
#include <Dialogs/ConnectionCount/ConnectionCountDialog.h>
#include <GUI/Model/Utils/SegmentationUtils.h>
#include <GUI/View/ViewState.h>
#include <Support/Settings/Settings.h>

// Qt
#include <QListWidgetItem>

using namespace ESPINA;
using namespace ESPINA::GUI::Model::Utils;
using namespace ESPINA::Support;

const QString SETTINGS_GROUP = "Synapse Connection Count Dialog";

//--------------------------------------------------------------------
ConnectionCountDialog::ConnectionCountDialog(Support::Context& context, QWidget* parent, Qt::WindowFlags flags)
: QDialog{parent, Qt::WindowFlags{Qt::WindowMinMaxButtonsHint|Qt::WindowCloseButtonHint}}
, WithContext(context)
{
  setupUi(this);

  updateList();

  connectSignals();

  ESPINA_SETTINGS(settings);

  settings.beginGroup(SETTINGS_GROUP);
  resize(settings.value("size", QSize (800, 800)).toSize());
  move  (settings.value("pos",  QPoint(200, 200)).toPoint());
  settings.endGroup();
}

//--------------------------------------------------------------------
void ConnectionCountDialog::closeEvent(QCloseEvent *event)
{
  ESPINA_SETTINGS(settings);

  settings.beginGroup(SETTINGS_GROUP);
  settings.setValue("size", size());
  settings.setValue("pos", pos());
  settings.endGroup();

  QDialog::closeEvent(event);
}

//--------------------------------------------------------------------
void ConnectionCountDialog::onItemDoubleClicked(QListWidgetItem *item)
{
  auto segmentation = reinterpret_cast<SegmentationAdapter *>(item->data(Qt::UserRole).toULongLong());

  if(segmentation) getViewState().focusViewOn(centroid(segmentation->bounds()));
}

//--------------------------------------------------------------------
void ConnectionCountDialog::connectSignals()
{
  for(auto list: {m_fullList, m_halfList, m_noneList})
  {
    connect(list, SIGNAL(itemDoubleClicked(QListWidgetItem *)), this, SLOT(onItemDoubleClicked(QListWidgetItem *)));
  }

  connect(getModel().get(), SIGNAL(segmentationsAdded(ViewItemAdapterSList)), this, SLOT(onSegmentationsAdded(ViewItemAdapterSList)));
  connect(getModel().get(), SIGNAL(segmentationsRemoved(ViewItemAdapterSList)), this, SLOT(updateList()));

  for(auto seg: getModel()->segmentations())
  {
    if(seg->category()->classificationName().startsWith("Dendrite") || seg->category()->classificationName().startsWith("Axon"))
    {
      connect(seg.get(), SIGNAL(outputModified()), this, SLOT(updateList()));
    }
  }
}

//--------------------------------------------------------------------
void ConnectionCountDialog::onSegmentationsAdded(ViewItemAdapterSList segmentations)
{
  for(auto seg: segmentations)
  {
    auto segmentation = std::dynamic_pointer_cast<SegmentationAdapter>(seg);

    if(segmentation &&
       (segmentation->category()->classificationName().startsWith("Dendrite") ||
        segmentation->category()->classificationName().startsWith("Axon")))
    {
      connect(seg.get(), SIGNAL(outputModified()), this, SLOT(updateList()));
    }
  }

  updateList();
}


//--------------------------------------------------------------------
void ConnectionCountDialog::addSegmentationToLists(const SegmentationAdapterSPtr segmentation)
{
  auto name = segmentation->data().toString();
  QPixmap pixmap(32,32);
  pixmap.fill(segmentation->category()->color());
  auto item = new QListWidgetItem(QIcon(pixmap), name);
  item->setData(Qt::UserRole, reinterpret_cast<unsigned long long>(segmentation.get()));

  auto connections = getModel()->connections(segmentation);
  switch(connections.count())
  {
    case 0:
      item->setToolTip(tr("%1 is unconnected").arg(name));
      m_noneList->addItem(item);
      break;
    case 1:
      item->setToolTip(tr("%1 is half connected.\nConnected to: %2").arg(name).arg(connections.first().item2->data().toString()));
      m_halfList->addItem(item);
      break;
    case 2:
      item->setToolTip(tr("%1 is fully connected.\nConnected to: %2\nConnected to: %3").arg(name).arg(connections.at(0).item2->data().toString()).arg(connections.at(1).item2->data().toString()));
      m_fullList->addItem(item);
      break;
    default:
      // more than 2 connections? show the error.
      {
        item->setIcon(QIcon("./espina/warning.svg"));
        auto text = tr("%1 has more than two connections!").arg(name);
        for(auto connection: connections)
        {
          text += tr("\nConnected to: %1").arg(connection.item2->data().toString());
        }
        item->setToolTip(text);
        m_fullList->addItem(item);
      }
      break;
  }
}

//--------------------------------------------------------------------
void ConnectionCountDialog::updateLabels()
{
  auto full = m_fullList->model()->rowCount();
  auto half = m_halfList->model()->rowCount();
  auto none = m_noneList->model()->rowCount();

  m_noneLabel->setText(tr("<b>%1</b>").arg(QString::number(none)));
  m_halfLabel->setText(tr("<b>%1</b>").arg(QString::number(half)));
  m_fullLabel->setText(tr("<b>%1</b>").arg(QString::number(full)));
  m_totalLabel->setText(tr("<b>%1</b>").arg(QString::number(full + half + none)));
}

//--------------------------------------------------------------------
void ConnectionCountDialog::updateList()
{
  m_noneList->clear();
  m_halfList->clear();
  m_fullList->clear();

  for(auto seg: getModel()->segmentations())
  {
    if(seg->category()->classificationName().startsWith("Synapse"))
    {
      addSegmentationToLists(seg);
    }
  }

  updateLabels();
}
