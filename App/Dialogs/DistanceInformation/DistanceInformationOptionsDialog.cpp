/*
 * Copyright (C) 2016, Rafael Juan Vicente Garcia <rafaelj.vicente@gmail.com>
 *
 * This file is part of ESPINA.
 *
 * ESPINA is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

// ESPINA
#include "DistanceInformationOptionsDialog.h"
#include <GUI/Dialogs/DefaultDialogs.h>
#include <GUI/Model/CategoryAdapter.h>
#include <GUI/Widgets/CategorySelector.h>
#include <Support/Context.h>

using namespace ESPINA;
using namespace ESPINA::GUI::Widgets;

//----------------------------------------------------------------------------
DistanceInformationOptionsDialog::DistanceInformationOptionsDialog(Support::Context &context)
: QDialog{GUI::DefaultDialogs::defaultParentWidget(), Qt::WindowStaysOnTopHint}
{
  setupUi(this);

  m_category = new CategorySelector{context.model(), nullptr};
  m_category->setEnabled(false);

  connect(m_categoryCheck, SIGNAL(toggled(bool)), m_category, SLOT(setEnabled(bool)));

  m_constraintsBox->layout()->addWidget(m_category);

  connect(m_minDistance, SIGNAL(valueChanged(double)), this, SLOT(onMinimumValueChanged(double)));
  connect(m_maxDistance, SIGNAL(valueChanged(double)), this, SLOT(onMaximumValueChanged(double)));
  connect(m_minDistanceCheck, SIGNAL(toggled(bool)), this, SLOT(onMinimumCheckChanged(bool)));
  connect(m_maxDistanceCheck, SIGNAL(toggled(bool)), this, SLOT(onMaximumCheckChanged(bool)));
}

//----------------------------------------------------------------------------
DistanceInformationOptionsDialog::TableType DistanceInformationOptionsDialog::getTableType() const
{
  return (m_combinedTable->isChecked()) ? TableType::COMBINED : TableType::SINGLE;
}

//----------------------------------------------------------------------------
DistanceInformationOptionsDialog::DistanceType DistanceInformationOptionsDialog::getDistanceType() const
{
  return (m_surfaceOption->isChecked()) ? DistanceType::SURFACE : DistanceType::CENTROID;
}

//----------------------------------------------------------------------------
DistanceInformationOptionsDialog::Options DistanceInformationOptionsDialog::getOptions() const
{
  return Options{getDistanceType(),getMinimumDistance(), getMaximumDistance(), getTableType(), getCategory()};
}

//----------------------------------------------------------------------------
const double DistanceInformationOptionsDialog::getMaximumDistance() const
{
  return isMaximumDistanceEnabled() ? m_maxDistance->value() : 0;
}

//----------------------------------------------------------------------------
bool DistanceInformationOptionsDialog::isMaximumDistanceEnabled() const
{
  return m_maxDistanceCheck->isChecked();
}

//----------------------------------------------------------------------------
bool DistanceInformationOptionsDialog::isMinimumDistanceEnabled() const
{
  return m_minDistanceCheck->isChecked();
}

//----------------------------------------------------------------------------
const double DistanceInformationOptionsDialog::getMinimumDistance() const
{
  return isMinimumDistanceEnabled() ? m_minDistance->value() : 0;
}

//----------------------------------------------------------------------------
bool DistanceInformationOptionsDialog::isCategoryOptionEnabled() const
{
  return m_categoryCheck->isChecked();
}

//----------------------------------------------------------------------------
const CategoryAdapterSPtr DistanceInformationOptionsDialog::getCategory() const
{
  return isCategoryOptionEnabled() ? m_category->selectedCategory() : CategoryAdapterSPtr();
}

//----------------------------------------------------------------------------
void DistanceInformationOptionsDialog::onMinimumValueChanged(double value)
{
  if((m_maxDistanceCheck->isChecked()) && (value > m_maxDistance->value()))
  {
    m_minDistance->blockSignals(true);
    m_minDistance->setValue(m_maxDistance->value());
    m_minDistance->blockSignals(false);
  }
}

//----------------------------------------------------------------------------
void DistanceInformationOptionsDialog::onMaximumValueChanged(double value)
{
  if((m_minDistanceCheck->isChecked()) && (value < m_minDistance->value()))
  {
    m_maxDistance->blockSignals(true);
    m_maxDistance->setValue(m_minDistance->value());
    m_maxDistance->blockSignals(false);
  }
}

//----------------------------------------------------------------------------
void DistanceInformationOptionsDialog::onMinimumCheckChanged(bool value)
{
  if(value)
  {
    if(m_maxDistanceCheck->isChecked() && (m_minDistance->value() > m_maxDistance->value()))
    {
      m_minDistance->blockSignals(true);
      m_minDistance->setValue(m_maxDistance->value());
      m_minDistance->blockSignals(false);
    }
  }
}

//----------------------------------------------------------------------------
void DistanceInformationOptionsDialog::onMaximumCheckChanged(bool value)
{
  if(value)
  {
    if(m_minDistanceCheck->isChecked() && (m_minDistance->value() > m_maxDistance->value()))
    {
      m_maxDistance->blockSignals(true);
      m_maxDistance->setValue(m_minDistance->value());
      m_maxDistance->blockSignals(false);
    }
  }
}
