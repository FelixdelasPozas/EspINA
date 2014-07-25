/*
 
 Copyright (C) 2014 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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
#include "Dialogs/AdaptiveEdges/AdaptiveEdgesDialog.h"

// Qt
#include <QDialog>
#include <QRadioButton>
#include <qpicture.h>
#include <qpaintengine.h>
#include <qbitmap.h>

using namespace ESPINA;

//-----------------------------------------------------------------------------
AdaptiveEdgesDialog::AdaptiveEdgesDialog(QWidget *parent)
: QDialog(parent)
, m_adaptiveEdgesEnabled(false)
, m_backgroundColor(0)
, m_threshold(50)
{
  setupUi(this);

  m_adaptiveEdgesEnabled = false;

  radioStackEdges->setChecked(true);
  radioImageEdges->setChecked(false);

  blackButton->setEnabled(false);
  colorBox  ->setEnabled(false);
  whiteButton->setEnabled(false);

  thresholdBox  ->setEnabled(false);
  thresholdLabel->setEnabled(false);

  connect(radioStackEdges, SIGNAL(toggled(bool)),
          this,            SLOT(radioChanged(bool)));
  connect(radioImageEdges, SIGNAL(toggled(bool)),
          this,            SLOT(radioChanged(bool)));

  colorBox->setValue(m_backgroundColor);
  thresholdBox->setValue(m_threshold);

  connect(colorBox, SIGNAL(valueChanged(int)),
          this,     SLOT(bgColorChanged(int)));
  connect(thresholdBox, SIGNAL(valueChanged(int)),
          this,         SLOT(thresholdChanged(int)));
  connect(blackButton, SIGNAL(clicked(bool)),
          this, SLOT(setBlackBgColor()));
  connect(whiteButton, SIGNAL(clicked(bool)),
          this, SLOT(setWhiteBgColor()));

}

//------------------------------------------------------------------------
void AdaptiveEdgesDialog::radioChanged(bool value)
{
  if (sender() == radioStackEdges)
    radioImageEdges->setChecked(!value);
  else
    radioStackEdges->setChecked(!value);

  m_adaptiveEdgesEnabled = radioImageEdges->isChecked();

  blackButton->setEnabled(m_adaptiveEdgesEnabled);
  colorBox  ->setEnabled(m_adaptiveEdgesEnabled);
  whiteButton->setEnabled(m_adaptiveEdgesEnabled);

  thresholdLabel->setEnabled(m_adaptiveEdgesEnabled);
  thresholdBox->setEnabled(m_adaptiveEdgesEnabled);
}

//------------------------------------------------------------------------
void AdaptiveEdgesDialog::bgColorChanged(int value)
{
  m_backgroundColor = value;

  QPixmap image(":espina/edges-image.png");
  QPixmap bg(image.size());
  bg.fill(QColor(value, value, value));
  image.setMask(image.createMaskFromColor(Qt::black, Qt::MaskInColor));
  QPainter painter(&bg);
  painter.drawPixmap(0,0, image);

  adaptiveExample->setPixmap(bg);
}

//------------------------------------------------------------------------
void AdaptiveEdgesDialog::thresholdChanged(int value)
{
  m_threshold = thresholdBox->value();
}

//------------------------------------------------------------------------
void AdaptiveEdgesDialog::setBlackBgColor()
{
  colorBox->setValue(0);
}

//------------------------------------------------------------------------
void AdaptiveEdgesDialog::setWhiteBgColor()
{
  colorBox->setValue(255);
}
