/*
 <one line to give the program's name and a brief idea of what it does.>
 Copyright (C) 2013 Félix de las Pozas Álvarez <felixdelaspozas@gmail.com>

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
#include "Dialogs/TypeDialog.h"

// Qt
#include <QDialog>
#include <QRadioButton>

using namespace EspINA;
using namespace EspINA::CF;

//-----------------------------------------------------------------------------
TypeDialog::TypeDialog(QWidget *parent)
: QDialog(parent)
, m_type(CF::ADAPTIVE)
{
  setupUi(this);

  adaptiveRadio->setChecked(true);
  ortogonalRadio->setChecked(false);

  balckLabel->setEnabled(true);
  colorBox  ->setEnabled(true);
  whiteLabel->setEnabled(true);

  thresholdBox  ->setEnabled(true);
  thresholdLabel->setEnabled(true);

  connect(adaptiveRadio,   SIGNAL(toggled(bool)),
          this,            SLOT(radioChanged(bool)));
  connect(ortogonalRadio,SIGNAL(toggled(bool)),
          this,            SLOT(radioChanged(bool)));
}

//------------------------------------------------------------------------
void TypeDialog::radioChanged(bool value)
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