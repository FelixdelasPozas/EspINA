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
#include <GUI/Widgets/HueSelector.h>

// Qt
#include <QSet>
#include <QMessageBox>
#include <QtGui>

using namespace ESPINA;
using namespace ESPINA::GUI::Widgets;

//--------------------------------------------------------------------
ConnectionCriteriaDialog::ConnectionCriteriaDialog(const ModelAdapterSPtr model, const QStringList& criteria, QWidget* parent)
: QDialog         {parent}
, m_criteria      {criteria}
, m_classification{nullptr}
{
  setupUi(this);

  createWidgets(model);

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
const short ConnectionCriteriaDialog::validColor() const
{
  auto selector = qobject_cast<HueSelector *>(m_validLayout->itemAt(2)->widget());
  return selector->hueValue();
}

//--------------------------------------------------------------------
const short ConnectionCriteriaDialog::invalidColor() const
{
  auto selector = qobject_cast<HueSelector *>(m_invalidLayout->itemAt(2)->widget());
  return selector->hueValue();
}

//--------------------------------------------------------------------
const short ConnectionCriteriaDialog::unconnectedColor() const
{
  auto selector = qobject_cast<HueSelector *>(m_unconnectedLayout->itemAt(2)->widget());
  return selector->hueValue();
}

//--------------------------------------------------------------------
const short ConnectionCriteriaDialog::incompleteColor() const
{
  auto selector = qobject_cast<HueSelector *>(m_incompleteLayout->itemAt(2)->widget());
  return selector->hueValue();
}

//--------------------------------------------------------------------
const void ConnectionCriteriaDialog::setValidColor(const short hue) const
{
  auto selector = qobject_cast<HueSelector *>(m_validLayout->itemAt(2)->widget());
  selector->setHueValue(hue);
}

//--------------------------------------------------------------------
const void ConnectionCriteriaDialog::setInvalidColor(const short hue) const
{
  auto selector = qobject_cast<HueSelector *>(m_invalidLayout->itemAt(2)->widget());
  selector->setHueValue(hue);
}

//--------------------------------------------------------------------
const void ConnectionCriteriaDialog::setUnconnectedColor(const short hue) const
{
  auto selector = qobject_cast<HueSelector *>(m_unconnectedLayout->itemAt(2)->widget());
  selector->setHueValue(hue);
}

//--------------------------------------------------------------------
const void ConnectionCriteriaDialog::setIncompleteColor(const short hue) const
{
  auto selector = qobject_cast<HueSelector *>(m_incompleteLayout->itemAt(2)->widget());
  selector->setHueValue(hue);
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

//--------------------------------------------------------------------
void ConnectionCriteriaDialog::createWidgets(const ModelAdapterSPtr model)
{
  if(model)
  {
    m_classification = model->classification();

    m_selector = new CategorySelector(model, this);
    m_layout->insertWidget(0, m_selector, 1);
  }

  for(auto layout: {m_validLayout, m_invalidLayout, m_unconnectedLayout, m_incompleteLayout})
  {
    auto hueSelector = new HueSelector(this);
    hueSelector->reserveInitialValue(false);

    connect(hueSelector, SIGNAL(newHsv(int, int, int)), this, SLOT(onHueMoved(int)));

    layout->addWidget(hueSelector, 1);
  }
}

//--------------------------------------------------------------------
void ConnectionCriteriaDialog::onHueMoved(int value)
{
  auto object = qobject_cast<HueSelector *>(sender());
  if(object)
  {
    QList<QHBoxLayout *> layouts{m_validLayout, m_invalidLayout, m_unconnectedLayout, m_incompleteLayout};
    QList<QLabel *>      labels {m_validColor,  m_invalidColor,  m_unconnectedColor,  m_incompleteColor};

    for(auto layout: layouts)
    {
      auto widget = qobject_cast<HueSelector *>(layout->itemAt(2)->widget());
      if(object == widget)
      {
        auto label = labels.at(layouts.indexOf(layout));
        label->setAutoFillBackground(true);
        QPalette pal = label->palette();
        pal.setColor(QPalette::Window, QColor::fromHsv(value, 255, 255));
        label->setPalette(pal);
        return;
      }
    }
  }
}

//--------------------------------------------------------------------
void ConnectionCriteriaDialog::showColors(const bool enable)
{
  m_colorGroup->setVisible(enable);

  auto size = sizeHint();
  size.setWidth(size.width() + 30);
  size.setHeight(size.height() + 50);
  setMaximumSize(size);
  setMinimumSize(size);
}
