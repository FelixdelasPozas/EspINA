/*

 Copyright (C) 2015 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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

#include <Dialogs/HueSelector/HueSelector.h>
#include <GUI/Dialogs/DefaultDialogs.h>

using namespace ESPINA;
using namespace ESPINA::GUI;

//------------------------------------------------------------------------
HueSelectorDialog::HueSelectorDialog(const Hue value)
: QDialog(DefaultDialogs::defaultParentWidget())
, m_hue{ value }
{
  setupUi(this);
  this->setWindowTitle(QObject::tr("Hue Selection"));

  m_selector = new HueSelector();
  m_selector->setFixedHeight(25);
  m_selector->reserveInitialValue(true);
  m_selector->setHueValue(value);
  m_selector->setVisible(true);
  m_hueSpinbox->setValue(value);
  m_hueSpinbox->setMaximum(359);
  m_layout->addWidget(m_selector, 1);

  connect(m_selector, SIGNAL(newHsv(int, int, int)), this, SLOT(onSelectorValueChanged(int, int, int)));

  connect(m_hueSpinbox, SIGNAL(valueChanged(int)), this, SLOT(onSpinboxValueChanged(int)));
}

//------------------------------------------------------------------------
HueSelectorDialog::~HueSelectorDialog()
{
}

//------------------------------------------------------------------------
void HueSelectorDialog::onSelectorValueChanged(int h, int s, int v)
{
  m_selector->blockSignals(true);
  m_hueSpinbox->setValue(h);
  m_selector->blockSignals(false);
}

//------------------------------------------------------------------------
void HueSelectorDialog::onSpinboxValueChanged(int h)
{
  m_hueSpinbox->blockSignals(true);
  m_selector->setHueValue(h);
  m_hueSpinbox->blockSignals(false);
}

//------------------------------------------------------------------------
Hue HueSelectorDialog::hueValue() const
{
  return m_hueSpinbox->value();
}
