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
using ESPINA::GUI::DefaultDialogs;
using DistType = ESPINA::DistanceInformationOptionsDialog::DistanceInformationType;
using DistOpts = ESPINA::DistanceInformationOptionsDialog::DistanceInformationOptions;

//----------------------------------------------------------------------------
DistanceInformationOptionsDialog::DistanceInformationOptionsDialog()
: QDialog {DefaultDialogs::defaultParentWidget(), Qt::WindowStaysOnTopHint}
{
  setupUi(this);

  setWindowIconText(":/espina/Espina.svg");

  connect(m_checkBox_maxDistance, SIGNAL(stateChanged(int)),
          this,           SLOT(maximumDistanceStateChanged(int)));
}

//----------------------------------------------------------------------------
DistOpts DistanceInformationOptionsDialog::getDistanceInformationOptions() const
{
  return DistOpts() = {getDistanceType(),isMaximumDistanceEnabled(),getMaximumDistance()};
}

//----------------------------------------------------------------------------
DistType DistanceInformationOptionsDialog::getDistanceType() const
{
  return (m_radioButton_surface->isChecked()) ? DistType::CENTROID : DistType::SURFACE;
}

//----------------------------------------------------------------------------
double DistanceInformationOptionsDialog::getMaximumDistance() const
{
  return m_doubleSpinBox_maxDistance->value();
}

//----------------------------------------------------------------------------
bool DistanceInformationOptionsDialog::isCentroidOptionSelected() const
{
  return m_radioButton_centroid->isChecked();
}

//----------------------------------------------------------------------------
bool DistanceInformationOptionsDialog::isMaximumDistanceEnabled() const
{
  return m_checkBox_maxDistance->isChecked();
}


//----------------------------------------------------------------------------
bool DistanceInformationOptionsDialog::isSurfaceOptionSelected() const
{
  return m_radioButton_surface->isChecked();
}

//----------------------------------------------------------------------------
void DistanceInformationOptionsDialog::maximumDistanceStateChanged(int state)
{
  m_doubleSpinBox_maxDistance->setEnabled(state != Qt::Unchecked);
}
