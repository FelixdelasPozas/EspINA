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
#include "CreateCategoryDialog.h"
#include <GUI/Dialogs/DefaultDialogs.h>

using namespace ESPINA;
using namespace GUI;

//--------------------------------------------------------------------
CreateCategoryDialog::CreateCategoryDialog()
: QDialog{DefaultDialogs::defaultParentWidget(), Qt::WindowFlags{Qt::WindowCloseButtonHint}}
{
  setupUi(this);

  setWindowTitle(tr("Create Category"));

  m_selector = new HueSelector();
  m_selector->setFixedHeight(25);
  m_selector->reserveInitialValue(false);
  m_selector->setHueValue(0);
  m_selector->setVisible(true);

  m_hueSpinbox->setValue(0);
  m_hueSpinbox->setMaximum(359);

  m_colorLayout->insertWidget(1, m_selector, 1);

  connect(m_selector, SIGNAL(newHsv(int, int, int)),
          this,       SLOT(onSelectorValueChanged(int, int, int)));

  connect(m_hueSpinbox, SIGNAL(valueChanged(int)),
          this,         SLOT(onSpinboxValueChanged(int)));
}

//--------------------------------------------------------------------
void CreateCategoryDialog::setOperationText(const QString& text)
{
  m_operation->setText(text);
}

//--------------------------------------------------------------------
void CreateCategoryDialog::setCategoryName(const QString& name)
{
  m_name->setText(name);
}

//--------------------------------------------------------------------
void CreateCategoryDialog::setColor(const QColor& color)
{
  m_selector->setHueValue(color.hsvHue());
}

//--------------------------------------------------------------------
void CreateCategoryDialog::setROI(const Vector3<long long>& values)
{
  m_xROI->setValue(values[0]);
  m_yROI->setValue(values[1]);
  m_zROI->setValue(values[2]);
}

//--------------------------------------------------------------------
const QString CreateCategoryDialog::categoryName() const
{
  return m_name->text();
}

//--------------------------------------------------------------------
const QColor CreateCategoryDialog::categoryColor() const
{
  return QColor::fromHsv(m_hueSpinbox->value(), 255,255);
}

//--------------------------------------------------------------------
const Vector3<long long> CreateCategoryDialog::ROI() const
{
  return Vector3<long long>{m_xROI->value(), m_yROI->value(), m_zROI->value()};
}

//--------------------------------------------------------------------
void CreateCategoryDialog::onSelectorValueChanged(int h, int s, int v)
{
  m_hueSpinbox->blockSignals(true);
  m_hueSpinbox->setValue(h);
  m_hueSpinbox->blockSignals(false);
}

//--------------------------------------------------------------------
void CreateCategoryDialog::onSpinboxValueChanged(int h)
{
  m_selector->blockSignals(true);
  m_selector->setHueValue(h);
  m_selector->blockSignals(false);
}
