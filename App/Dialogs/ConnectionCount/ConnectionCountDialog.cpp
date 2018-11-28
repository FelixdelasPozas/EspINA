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
#include <Core/Utils/ListUtils.hxx>
#include <Dialogs/ConnectionCount/ConnectionCountDialog.h>
#include <GUI/Model/Utils/SegmentationUtils.h>
#include <GUI/View/ViewState.h>
#include <Support/Settings/Settings.h>
#include "ConnectionCriteriaDialog.h"

// Qt
#include <QListWidgetItem>

using namespace ESPINA;
using namespace ESPINA::GUI::Model::Utils;
using namespace ESPINA::Support;

const QString SETTINGS_GROUP = "Synapse Connection Count Dialog";
const QString CRITERIA_KEY   = "Connection Criteria";

//--------------------------------------------------------------------
ConnectionCountDialog::ConnectionCountDialog(Support::Context& context, QWidget* parent, Qt::WindowFlags flags)
: QDialog{parent, Qt::WindowFlags{Qt::WindowMinMaxButtonsHint|Qt::WindowCloseButtonHint}}
, WithContext(context)
{
  setupUi(this);

  restoreSettings();

  updateCriteriaLabel();

  updateList();

  connectSignals();
}

//--------------------------------------------------------------------
void ConnectionCountDialog::closeEvent(QCloseEvent *event)
{
  saveSettings();

  QDialog::closeEvent(event);
}

//--------------------------------------------------------------------
void ConnectionCountDialog::onItemDoubleClicked(QListWidgetItem *item)
{
  auto segmentation = reinterpret_cast<SegmentationAdapter *>(item->data(Qt::UserRole).toULongLong());

  if(segmentation) getViewState().focusViewOn(centroid(segmentation->bounds()));
}

//--------------------------------------------------------------------
void ConnectionCountDialog::onItemClicked(QListWidgetItem *item)
{
  auto segmentation = reinterpret_cast<SegmentationAdapter *>(item->data(Qt::UserRole).toULongLong());

  if(segmentation)
  {
    SegmentationAdapterList list;
    list << segmentation;

    getSelection()->set(list);
    getViewState().refresh();
  }
}

//--------------------------------------------------------------------
void ConnectionCountDialog::connectSignals()
{
  for(auto list: {m_fullList, m_halfList, m_noneList, m_invalidList})
  {
    connect(list, SIGNAL(itemDoubleClicked(QListWidgetItem *)), this, SLOT(onItemDoubleClicked(QListWidgetItem *)));
    connect(list, SIGNAL(itemClicked(QListWidgetItem *)), this, SLOT(onItemClicked(QListWidgetItem *)));
  }

  connect(getModel().get(), SIGNAL(segmentationsAdded(ViewItemAdapterSList)), this, SLOT(onSegmentationsAdded(ViewItemAdapterSList)));
  connect(getModel().get(), SIGNAL(segmentationsRemoved(ViewItemAdapterSList)), this, SLOT(updateList()));

  for(auto segmentation: getModel()->segmentations())
  {
    if(segmentation->category()->classificationName().startsWith("Dendrite", Qt::CaseInsensitive) ||
       segmentation->category()->classificationName().startsWith("Axon", Qt::CaseInsensitive))
    {
      connect(segmentation.get(), SIGNAL(outputModified()), this, SLOT(updateList()));
    }
  }

  connect(m_changeButton, SIGNAL(pressed()), this, SLOT(onChangeButtonPressed()));
}

//--------------------------------------------------------------------
void ConnectionCountDialog::onSegmentationsAdded(ViewItemAdapterSList segmentations)
{
  for(auto seg: segmentations)
  {
    auto segmentation = std::dynamic_pointer_cast<SegmentationAdapter>(seg);

    if(segmentation && (segmentation->category()->classificationName().startsWith("Dendrite", Qt::CaseInsensitive) ||
       segmentation->category()->classificationName().startsWith("Axon", Qt::CaseInsensitive)))
    {
      connect(segmentation.get(), SIGNAL(outputModified()), this, SLOT(updateList()));
    }
  }

  updateList();
}

//--------------------------------------------------------------------
void ConnectionCountDialog::addSegmentationsToLists()
{
  QList<QListWidgetItem *> noneList, halfList, fullList, invalidList;

  /** helper method to create a text string from the connections. */
  auto connectionsToText = [](const ConnectionList &connections)
  {
    QString result;

    for(auto connection: connections)
    {
      result += tr("\nConnected to: %1").arg(connection.item2->data().toString());
    }

    return result;
  };

  /** helper method that returns true if the given connections matches the criteria. */
  auto isValid = [](const ConnectionList &connections, const QStringList &criteria)
  {
    auto current = criteria;
    for(auto connection: connections)
    {
      if(current.isEmpty()) return false;

      auto category = connection.item2->category()->classificationName();
      if(current.contains(category))
      {
        current.removeOne(category);
      }
      else
      {
        for(auto criteriaCategory: current)
        {
          if(category.startsWith(criteriaCategory, Qt::CaseInsensitive))
          {
            current.removeOne(criteriaCategory);
            break;
          }
        }
      }
    }

    if(current.isEmpty()) return true;
    return false;
  };

  for(auto segmentation: getModel()->segmentations())
  {
    if(segmentation->category()->classificationName().startsWith("Synapse", Qt::CaseInsensitive))
    {
      auto criteria = m_criteria;

      auto name = segmentation->data().toString();
      QPixmap pixmap(32,32);
      pixmap.fill(segmentation->category()->color());
      auto item = new QListWidgetItem(QIcon(pixmap), name);
      item->setData(Qt::UserRole, reinterpret_cast<unsigned long long>(segmentation.get()));

      auto connections = getModel()->connections(segmentation);

      if(connections.size() == 0 || m_criteria.isEmpty())
      {
        item->setToolTip(tr("%1 is unconnected.").arg(name));
        noneList << item;
      }
      else
      {
        if(connections.size() < m_criteria.size())
        {
          auto text = tr("%1 is incomplete.").arg(name);
          text += connectionsToText(connections);
          item->setToolTip(text);
          halfList << item;
        }
        else
        {
          if(connections.size() == m_criteria.size() && isValid(connections, m_criteria))
          {
            auto text = tr("%1 is valid.").arg(name);
            text += connectionsToText(connections);
            item->setToolTip(text);
            fullList << item;
          }
          else
          {
            auto text = tr("%1 is invalid.").arg(name);
            text += connectionsToText(connections);
            item->setIcon(QIcon(":/espina/warning.svg"));
            item->setBackgroundColor(Qt::red);
            item->setTextColor(Qt::white);
            item->setToolTip(text);
            invalidList << item;
          }
        }
      }
    }
  }

  qSort(noneList.begin(), noneList.end(), Core::Utils::lessThan<QListWidgetItem *>);
  qSort(halfList.begin(), halfList.end(), Core::Utils::lessThan<QListWidgetItem *>);
  qSort(fullList.begin(), fullList.end(), Core::Utils::lessThan<QListWidgetItem *>);
  qSort(invalidList.begin(), invalidList.end(), Core::Utils::lessThan<QListWidgetItem *>);

  for(auto list: {m_noneList, m_halfList, m_fullList, m_invalidList})
  {
    list->blockSignals(true);
  }

  for(int i = 0; i < noneList.size(); ++i) m_noneList->addItem(noneList.at(i));
  for(int i = 0; i < halfList.size(); ++i) m_halfList->addItem(halfList.at(i));
  for(int i = 0; i < fullList.size(); ++i) m_fullList->addItem(fullList.at(i));
  for(int i = 0; i < invalidList.size(); ++i) m_invalidList->addItem(invalidList.at(i));

  for(auto list: {m_noneList, m_halfList, m_fullList, m_invalidList})
  {
    list->blockSignals(false);
    list->repaint();
  }
}

//--------------------------------------------------------------------
void ConnectionCountDialog::updateLabels()
{
  auto full = m_fullList->model()->rowCount();
  auto half = m_halfList->model()->rowCount();
  auto none = m_noneList->model()->rowCount();
  auto invalid = m_invalidList->model()->rowCount();

  m_noneLabel->setText(tr("<b>%1</b>").arg(QString::number(none)));
  m_halfLabel->setText(tr("<b>%1</b>").arg(QString::number(half)));
  m_fullLabel->setText(tr("<b>%1</b>").arg(QString::number(full)));
  m_invalidLabel->setText(tr("<b>%1</b>").arg(QString::number(invalid)));
  m_totalLabel->setText(tr("<b>%1</b>").arg(QString::number(full + half + none + invalid)));
}

//--------------------------------------------------------------------
void ConnectionCountDialog::updateList()
{
  m_noneList->clear();
  m_halfList->clear();
  m_fullList->clear();
  m_invalidList->clear();

  addSegmentationsToLists();

  updateLabels();

  updateInvalidVisibility();
}

//--------------------------------------------------------------------
void ConnectionCountDialog::updateInvalidVisibility()
{
  auto isVisible = m_invalidList->model()->rowCount() != 0;

  m_invalidList->setVisible(isVisible);
  m_invalidLabel->setVisible(isVisible);
  m_invalidTextLabel->setVisible(isVisible);
}

//--------------------------------------------------------------------
void ConnectionCountDialog::onChangeButtonPressed()
{
  ConnectionCriteriaDialog dialog{getModel(), m_criteria, this};

  if((dialog.exec() == QDialog::Accepted) && (m_criteria != dialog.criteria()))
  {
    m_criteria = dialog.criteria();

    updateCriteriaLabel();

    updateList();
  }
}

//--------------------------------------------------------------------
void ConnectionCountDialog::restoreSettings()
{
  ESPINA_SETTINGS(settings);

  settings.beginGroup(SETTINGS_GROUP);
  m_criteria = settings.value(CRITERIA_KEY, QStringList()).toStringList();
  resize(settings.value("size", QSize (800, 800)).toSize());
  move  (settings.value("pos",  QPoint(200, 200)).toPoint());
  settings.endGroup();
}

//--------------------------------------------------------------------
void ConnectionCountDialog::saveSettings()
{
  ESPINA_SETTINGS(settings);

  settings.beginGroup(SETTINGS_GROUP);
  settings.setValue("size", size());
  settings.setValue("pos", pos());
  settings.setValue(CRITERIA_KEY, m_criteria);
  settings.endGroup();
  settings.sync();
}

//--------------------------------------------------------------------
void ConnectionCountDialog::updateCriteriaLabel()
{
  auto text = ConnectionCriteriaDialog::criteriaToText(m_criteria, getModel()->classification());
  text.replace("<br>", ", ");

  m_criteriaLabel->setText(text);
}
