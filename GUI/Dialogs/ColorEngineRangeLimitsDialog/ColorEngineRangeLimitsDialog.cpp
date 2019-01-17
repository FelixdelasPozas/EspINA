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
#include "ColorEngineRangeLimitsDialog.h"

// C++
#include <numeric>
#include <QDebug>

using namespace ESPINA;
using namespace ESPINA::GUI;

//--------------------------------------------------------------------
ColorEngineRangeLimitsDialog::ColorEngineRangeLimitsDialog(const double min, const double max, const QString &property, QWidget* parent, Qt::WindowFlags flags)
: QDialog{parent, flags}
{
  setupUi(this);

  m_min->setValue(min);
  m_max->setValue(max);
  m_property->setText(tr("%1").arg(property));

  connectSignals();
}

//--------------------------------------------------------------------
void ColorEngineRangeLimitsDialog::onMinValueChanged(double value)
{
  auto min = std::min(m_max->value(), value);

  m_max->setRange(min, std::numeric_limits<double>::max());

  m_min->blockSignals(true);
  m_min->setValue(min);
  m_min->blockSignals(false);
}

//--------------------------------------------------------------------
void ColorEngineRangeLimitsDialog::onMaxValueChanged(double value)
{
  auto max = std::max(m_min->value(), value);

  m_min->setRange(std::numeric_limits<double>::min(), max);

  m_max->blockSignals(true);
  m_max->setValue(max);
  m_max->blockSignals(false);
}

//--------------------------------------------------------------------
void ColorEngineRangeLimitsDialog::connectSignals()
{
  connect(m_min, SIGNAL(valueChanged(double)), this, SLOT(onMinValueChanged(double)));
  connect(m_max, SIGNAL(valueChanged(double)), this, SLOT(onMaxValueChanged(double)));
}
