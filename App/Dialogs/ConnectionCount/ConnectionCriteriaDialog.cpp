/*

 Copyright (C) 2018 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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
#include <Dialogs/ConnectionCount/ConnectionCriteriaDialog.h>
#include <GUI/Widgets/CategorySelector.h>

// Qt
#include <QSet>
#include <QMessageBox>

using namespace ESPINA;
using namespace ESPINA::GUI::Widgets;

//--------------------------------------------------------------------
ConnectionCriteriaDialog::ConnectionCriteriaDialog(const ModelAdapterSPtr model, const QStringList& criteria, QWidget* parent)
: QDialog         {parent}
, m_criteria      {criteria}
, m_classification{nullptr}
{
  setupUi(this);

  if(model) m_classification = model->classification();

  m_selector = new CategorySelector(model, this);
  m_layout->insertWidget(0, m_selector, 1);

  connectSignals();

  updateCriteria();
}

//--------------------------------------------------------------------
bool ConnectionCriteriaDialog::isAmbiguous() const
{
  for(auto category: m_criteria)
  {
    for(auto other: m_criteria)
    {
      if(category == other) continue;
      if(other.startsWith(category)) return true;
    }
  }

  return false;
}

//--------------------------------------------------------------------
void ConnectionCriteriaDialog::onAddPressed()
{
  m_criteria << m_selector->selectedCategory()->classificationName();

  updateCriteria();
}

//--------------------------------------------------------------------
void ConnectionCriteriaDialog::onRemovePressed()
{
  m_criteria.removeOne(m_selector->selectedCategory()->classificationName());

  updateCriteria();
}

//--------------------------------------------------------------------
void ConnectionCriteriaDialog::onClearPressed()
{
  m_criteria.clear();

  updateCriteria();
}

//--------------------------------------------------------------------
void ConnectionCriteriaDialog::connectSignals()
{
  connect(m_add, SIGNAL(pressed()), this, SLOT(onAddPressed()));
  connect(m_erase, SIGNAL(pressed()), this, SLOT(onRemovePressed()));
  connect(m_clear, SIGNAL(pressed()), this, SLOT(onClearPressed()));
  connect(m_selector, SIGNAL(currentIndexChanged(int)), this, SLOT(updateGUI()));
}

//--------------------------------------------------------------------
void ConnectionCriteriaDialog::updateGUI()
{
  auto current = m_selector->selectedCategory()->classificationName();

  m_erase->setEnabled(m_criteria.contains(current));
}

//--------------------------------------------------------------------
void ConnectionCriteriaDialog::accept()
{
  if(isAmbiguous())
  {
    QMessageBox msgBox{this};
    msgBox.setWindowIcon(QIcon(":/espina/connection.svg"));
    msgBox.setWindowTitle(tr("Ambiguous criteria"));
    msgBox.setIcon(QMessageBox::Icon::Warning);
    msgBox.setText(tr("Selected criteria is ambiguous and can create problems when validating connections.\n"
                      "Do you want to continue anyways?"));
    msgBox.setStandardButtons(QMessageBox::Yes|QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::No);

    if(msgBox.exec() == QMessageBox::No) return;
  }

  QDialog::accept();
}

//--------------------------------------------------------------------
const QString ESPINA::ConnectionCriteriaDialog::criteriaToText(const QStringList& criteria, const ClassificationAdapterSPtr classification)
{
  QString result{"None established."};

  if(!criteria.isEmpty())
  {
    auto criteriaSet = criteria.toSet();

    QStringList criteriaLabel;
    for(auto category: criteriaSet)
    {
      auto count = criteria.count(category);
      QString color;

      if(classification && classification->category(category))
      {
        color = classification->category(category)->color().name();
      }
      else
      {
        color = "black";
      }

      switch(count)
      {
        case 1:
          criteriaLabel << tr("A connection to <b><font color='%1'>%2</font></b>").arg(color).arg(category);
          break;
        default:
          criteriaLabel << tr("%1 connections to <b><font color='%2'>%3</font></b>").arg(count).arg(color).arg(category);
          break;
      }
    }

    if(!criteriaLabel.isEmpty())
    {
      result = criteriaLabel.join("<br>");
    }
  }

  return result;
}

//--------------------------------------------------------------------
void ConnectionCriteriaDialog::updateCriteria()
{
  updateGUI();

  m_criteria.sort();

  m_criteriaLabel->setText(criteriaToText(m_criteria, m_classification));

  if(isAmbiguous())
  {
    m_ambiguous->setText(tr("Current criteria is ambiguous:"));
  }
  else
  {
    m_ambiguous->setText(tr("Current criteria:"));
  }
}
