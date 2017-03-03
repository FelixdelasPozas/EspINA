/*
 *    
 *    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 *    This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

// ESPINA
#include "Dialogs/CFTypeSelectorDialog.h"

#include <Core/Analysis/Channel.h>
#include <GUI/Model/ModelAdapter.h>
#include <Extensions/ExtensionUtils.h>
#include <Extensions/EdgeDistances/ChannelEdges.h>
#include <Support/Context.h>

// Qt
#include <QDialog>
#include <QRadioButton>
#include <QStringListModel>

using namespace ESPINA;
using namespace ESPINA::Extensions;
using namespace ESPINA::CF;

//-----------------------------------------------------------------------------
CFTypeSelectorDialog::CFTypeSelectorDialog(Support::Context &context, QWidget *parent)
: QDialog  {parent}
, m_type   {CF::ADAPTIVE}
, m_proxy  {std::make_shared<ChannelProxy>(context.model())}
, m_stack  {nullptr}
, m_model  {context.model()}
, m_factory{context.factory()}
{
  setupUi(this);

  auto model = context.model();

  adaptiveRadio->setChecked(true);
  orthogonalRadio->setChecked(false);

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

  connect(orthogonalRadio, SIGNAL(toggled(bool)),
          this,           SLOT(radioChanged(bool)));

  connect(useCategoryConstraint, SIGNAL(toggled(bool)),
          categorySelector,      SLOT(setEnabled(bool)));

  categorySelector->setModel(model.get());
  categorySelector->setRootModelIndex(model->classificationRoot());

  for(auto channel: model->channels())
  {
    m_stackNames << channel->data().toString();
  }
  auto stackModel = new QStringListModel(m_stackNames);
  channelSelector->setModel(stackModel);

  connect(channelSelector, SIGNAL(activated(QModelIndex)),
          this,            SLOT(channelSelected()));

  connect(channelSelector, SIGNAL(activated(int)),
          this,            SLOT(channelSelected()));

  // use first channel as default to force CF type selection using edges extension.
  channelSelector->setCurrentIndex(0);
  channelSelected();
}

//------------------------------------------------------------------------
void CFTypeSelectorDialog::setType(CFType type)
{
  if (ORTOGONAL == type)
  {
    orthogonalRadio->setChecked(true);
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

      auto category = toCategoryAdapterPtr(item);

      constraint = category->classificationName();
    }
  }

  return constraint;
}


//------------------------------------------------------------------------
void CFTypeSelectorDialog::channelSelected()
{
  auto index = channelSelector->currentIndex();

  Q_ASSERT(0 <= index && index < m_stackNames.size());
  auto stackName = m_stackNames.at(index);

  ChannelAdapterPtr item = nullptr;
  for(auto channel: m_model->channels())
  {
    if(stackName == channel->data().toString())
    {
      item = channel.get();
      break;
    }
  }

  if (item && isChannel(item))
  {
    m_stack = item;

    auto edgesExtension = retrieveOrCreateStackExtension<ChannelEdges>(m_stack, m_factory);
    if (edgesExtension->useDistanceToBounds())
    {
      setType(CF::ORTOGONAL);
    }
    else
    {
      setType(CF::ADAPTIVE);
    }
  }
}

//------------------------------------------------------------------------
void CFTypeSelectorDialog::radioChanged(bool value)
{
  bool adaptiveChecked = sender() == adaptiveRadio;
  if (adaptiveChecked)
  {
    orthogonalRadio->setChecked(!value);
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
