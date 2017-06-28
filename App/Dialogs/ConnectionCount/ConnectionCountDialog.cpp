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
: QDialog{parent, flags}
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
  QPixmap pixmap(32,32);
  pixmap.fill(segmentation->category()->color());
  auto item = new QListWidgetItem(QIcon(pixmap), segmentation->data().toString());
  item->setData(Qt::UserRole, reinterpret_cast<unsigned long long>(segmentation.get()));

  switch(getModel()->connections(segmentation).count())
  {
    case 1:
      m_halfList->addItem(item);
      break;
    case 0:
      m_noneList->addItem(item);
      break;
    case 2:
    default:
      m_fullList->addItem(item);
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
