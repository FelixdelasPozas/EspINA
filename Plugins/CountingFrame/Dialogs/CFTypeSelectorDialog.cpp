/*
 <one line to give the program's name and a brief idea of what it does.>
 Copyright (C) 2013 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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

// EspINA
#include "Dialogs/CFTypeSelectorDialog.h"
#include <GUI/Model/ModelAdapter.h>
#include <Extensions/ExtensionUtils.h>
#include <Extensions/EdgeDistances/ChannelEdges.h>

// Qt
#include <QDialog>
#include <QRadioButton>

using namespace EspINA;
using namespace EspINA::CF;

//-----------------------------------------------------------------------------
CFTypeSelectorDialog::CFTypeSelectorDialog(ModelAdapterSPtr model, QWidget *parent)
: QDialog(parent)
, m_type(CF::ADAPTIVE)
, m_proxy(new ChannelProxy(model))
, m_channel(nullptr)
{
  setupUi(this);

  adaptiveRadio->setChecked(true);
  ortogonalRadio->setChecked(false);

  balckLabel->setEnabled(true);
  colorBox  ->setEnabled(true);
  whiteLabel->setEnabled(true);

  thresholdBox  ->setEnabled(true);
  thresholdLabel->setEnabled(true);

  balckLabel->setVisible(false);
  colorBox->setVisible(false);
  whiteLabel->setVisible(false);

  thresholdBox  ->setVisible(false);
  thresholdLabel->setVisible(false);

  connect(adaptiveRadio, SIGNAL(toggled(bool)),
          this,          SLOT(radioChanged(bool)));

  connect(ortogonalRadio, SIGNAL(toggled(bool)),
          this,           SLOT(radioChanged(bool)));

  connect(useCategoryConstraint, SIGNAL(toggled(bool)),
          categorySelector,      SLOT(setEnabled(bool)));

  categorySelector->setModel(model.get());
  categorySelector->setRootModelIndex(model->classificationRoot());

  channelSelector->setModel(m_proxy.get());

  connect(channelSelector, SIGNAL(activated(QModelIndex)),
          this, SLOT(channelSelected()));

  connect(channelSelector, SIGNAL(activated(int)),
          this, SLOT(channelSelected()));

  m_channelIndex = m_proxy->index(0, 0);
  m_channelIndex = m_channelIndex.child(0,0);

  channelSelector->setCurrentModelIndex(m_channelIndex);


}

//------------------------------------------------------------------------
void CFTypeSelectorDialog::setType(CFType type)
{
  if (ORTOGONAL == type)
  {
    ortogonalRadio->setChecked(true);
  }
  else
  {
    adaptiveRadio->setChecked(true);
  }
}

//------------------------------------------------------------------------
QString CFTypeSelectorDialog::categoryConstraint() const
{
  QString constraint;

  if (useCategoryConstraint->isChecked())
  {
    QModelIndex categoryyIndex = categorySelector->currentModelIndex();
    if (categoryyIndex.isValid())
    {
      auto item = itemAdapter(categoryyIndex);
      Q_ASSERT(isCategory(item));

      auto category = categoryPtr(item);

      constraint = category->classificationName();
    }
  }

  return constraint;
}


//------------------------------------------------------------------------
void CFTypeSelectorDialog::channelSelected()
{
  auto currentIndex = channelSelector->currentModelIndex();

  auto item = itemAdapter(currentIndex);

  if (!item || isSample(item))
  {
    currentIndex = currentIndex.child(0, 0);
    item = itemAdapter(currentIndex);
  }

  if (item && isChannel(item))
  {
    m_channelIndex = currentIndex;

    m_channel = channelPtr(item);

    auto edgesExtension = retrieveOrCreateExtension<ChannelEdges>(m_channel);

    if (edgesExtension->useDistanceToBounds())
    {
      setType(CF::ORTOGONAL);
    }
    else
    {
      setType(CF::ADAPTIVE);
    }
  }
//   else
//   {
//     channelSelector->setCurrentModelIndex(m_channelIndex);
//   }
}

//------------------------------------------------------------------------
void CFTypeSelectorDialog::radioChanged(bool value)
{
  bool adaptiveChecked = sender() == adaptiveRadio;
  if (adaptiveChecked)
  {
    ortogonalRadio->setChecked(!value);
  } else
  {
    adaptiveRadio->setChecked(!value);
  }
  balckLabel->setEnabled(adaptiveChecked);
  colorBox  ->setEnabled(adaptiveChecked);
  whiteLabel->setEnabled(adaptiveChecked);

  thresholdBox  ->setEnabled(adaptiveChecked);
  thresholdLabel->setEnabled(adaptiveChecked);

  m_type = adaptiveRadio->isChecked()?ADAPTIVE:ORTOGONAL;
}