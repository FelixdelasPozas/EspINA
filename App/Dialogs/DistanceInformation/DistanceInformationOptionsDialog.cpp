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

#include "DistanceInformationOptionsDialog.h"
#include <GUI/Dialogs/DefaultDialogs.h>

using namespace ESPINA;

//----------------------------------------------------------------------------
DistanceInformationOptionsDialog::DistanceInformationOptionsDialog()
: QDialog{GUI::DefaultDialogs::defaultParentWidget(), Qt::WindowStaysOnTopHint}
{
  setupUi(this);

  connect(m_maxDistanceCheck, SIGNAL(stateChanged(int)),
          this,               SLOT(onMaxDistanceCheckChanged(int)));
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
  return Options{getDistanceType(),getMaximumDistance(), getTableType()};
}

//----------------------------------------------------------------------------
double DistanceInformationOptionsDialog::getMaximumDistance() const
{
  return isMaximumDistanceEnabled() ? m_maxDistance->value() : 0;
}

//----------------------------------------------------------------------------
bool DistanceInformationOptionsDialog::isMaximumDistanceEnabled() const
{
  return m_maxDistanceCheck->isChecked();
}

//----------------------------------------------------------------------------
void DistanceInformationOptionsDialog::onMaxDistanceCheckChanged(int state)
{
  m_maxDistance->setEnabled(state == Qt::Checked);
}
