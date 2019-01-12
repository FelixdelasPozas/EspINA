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
#include "ColorEngineRangeDefinitionDialog.h"
#include <GUI/Widgets/ColorBar.h>
#include <GUI/Widgets/HueSelector.h>

using namespace ESPINA;
using namespace ESPINA::GUI;
using namespace ESPINA::GUI::Widgets;

//--------------------------------------------------------------------
ColorEngineRangeDefinitionDialog::ColorEngineRangeDefinitionDialog(QWidget *parent, Qt::WindowFlags flags)
: QDialog{parent, flags}
{
  setupUi(this);

  createWidgets();

  m_fromSpinBox->setMinimum(0);
  m_fromSpinBox->setMaximum(359);
  m_toSpinBox->setMinimum(0);
  m_toSpinBox->setMaximum(359);

  connectSignals();
}

//--------------------------------------------------------------------
const bool ColorEngineRangeDefinitionDialog::showRangeInViews() const
{
  return m_showRange->isChecked();
}

//--------------------------------------------------------------------
void ColorEngineRangeDefinitionDialog::onColorModified(int value)
{
  auto selector = qobject_cast<HueSelector *>(sender());
  auto spinBox  = qobject_cast<QSpinBox *>(sender());

  if((selector && selector == m_fromSelector) || (spinBox && spinBox == m_fromSpinBox))
  {
    QPixmap icon{24,24};
    icon.fill(QColor::fromHsv(value, 255,255));
    m_fromLabel->setPixmap(icon);
    m_fromSpinBox->blockSignals(true);
    m_fromSpinBox->setValue(value);
    m_fromSpinBox->blockSignals(false);
    m_fromSelector->blockSignals(true);
    m_fromSelector->setHueValue(value);
    m_fromSelector->blockSignals(false);
  }
  else
  {
    if((selector && selector == m_toSelector) || (spinBox && spinBox == m_toSpinBox))
    {
      QPixmap icon{24,24};
      icon.fill(QColor::fromHsv(value, 255,255));
      m_toLabel->setPixmap(icon);
      m_toSpinBox->blockSignals(true);
      m_toSpinBox->setValue(value);
      m_toSpinBox->blockSignals(false);
      m_toSelector->blockSignals(true);
      m_toSelector->setHueValue(value);
      m_toSelector->blockSignals(false);
    }
  }

  setRangeColors(m_fromSpinBox->value(), m_toSpinBox->value());
}

//--------------------------------------------------------------------
void ColorEngineRangeDefinitionDialog::setRangeColors(const int hueMinimum, const int hueMaximum)
{
  auto width = (size().width() - 4 * layout()->spacing())/2;
  auto height = 24;

  Utils::RangeHSV range{0,1};
  range.setMinimumColor(QColor::fromHsv(hueMinimum, 255,255));
  range.setMaximumColor(QColor::fromHsv(hueMaximum, 255,255));
  auto image = ColorBar::rangeImage(&range, width, height);
  m_customBar->setPixmap(QPixmap::fromImage(image));

  m_fromSpinBox->setValue(hueMinimum);
  m_toSpinBox->setValue(hueMaximum);
}

//--------------------------------------------------------------------
const int ColorEngineRangeDefinitionDialog::minimum() const
{
  return m_fromSpinBox->value();
}

//--------------------------------------------------------------------
const int ColorEngineRangeDefinitionDialog::maximum() const
{
  return m_toSpinBox->value();
}

//--------------------------------------------------------------------
void ColorEngineRangeDefinitionDialog::setShowRangeInViews(const bool value)
{
  m_showRange->setChecked(value);
}

//--------------------------------------------------------------------
void ColorEngineRangeDefinitionDialog::onWidgetsEnabled(int value)
{
  auto enabled = value == Qt::Checked;

  m_positionLabel    ->setEnabled(enabled);
  m_positionCombo    ->setEnabled(enabled);
  m_orientationLabel ->setEnabled(enabled);
  m_orientationCombo ->setEnabled(enabled);
  m_textPositionLabel->setEnabled(enabled);
  m_textPositionCombo->setEnabled(enabled);
  m_numlabelsLabel   ->setEnabled(enabled);
  m_labelsSpin       ->setEnabled(enabled);
  m_widthLabel       ->setEnabled(enabled);
  m_widthSlider      ->setEnabled(enabled);
  m_heightLabel      ->setEnabled(enabled);
  m_heightSlider     ->setEnabled(enabled);
  m_barRatioLabel    ->setEnabled(enabled);
  m_barRatioSlider   ->setEnabled(enabled);
  m_decimalsLabel    ->setEnabled(enabled);
  m_decimalsSpin     ->setEnabled(enabled);
}

//--------------------------------------------------------------------
void ColorEngineRangeDefinitionDialog::createWidgets()
{
  QPixmap blue{24,24};
  blue.fill(Qt::blue);
  m_fromLabel->setPixmap(blue);
  m_fromSpinBox->setValue(QColor{Qt::blue}.hue());

  QPixmap red{24,24};
  red.fill(Qt::red);
  m_toLabel->setPixmap(red);
  m_toSpinBox->setValue(QColor{Qt::red}.hue());

  auto width = (size().width() - 4 * layout()->spacing())/2;
  auto height = 24;

  Utils::RangeHSV range{0,1};
  auto image = ColorBar::rangeImage(&range, width, height);
  m_customBar->setPixmap(QPixmap::fromImage(image));

  m_fromSelector = new HueSelector{nullptr};
  m_fromSelector->reserveInitialValue(false);
  m_fromSelector->setFixedSize(QSize{width,height});
  m_fromSelector->setHueValue(QColor{Qt::blue}.hue());

  connect(m_fromSelector, SIGNAL(newHsv(int, int, int)), this, SLOT(onColorModified(int)));
  connect(m_fromSpinBox, SIGNAL(valueChanged(int)), this, SLOT(onColorModified(int)));
  connect(m_fromSelector, SIGNAL(newHsv(int, int, int)), this, SIGNAL(widgetsPropertiesModified()));

  m_gridLayout->addWidget(m_fromSelector, 1, 1);

  m_toSelector = new HueSelector{nullptr};
  m_toSelector->reserveInitialValue(false);
  m_toSelector->setFixedSize(QSize{width,height});
  m_toSelector->setHueValue(QColor{Qt::red}.hue());

  connect(m_toSelector, SIGNAL(newHsv(int, int, int)), this, SLOT(onColorModified(int)));
  connect(m_toSpinBox, SIGNAL(valueChanged(int)), this, SLOT(onColorModified(int)));
  connect(m_toSelector, SIGNAL(newHsv(int, int, int)), this, SIGNAL(widgetsPropertiesModified()));

  m_gridLayout->addWidget(m_toSelector, 2, 1);
}

//--------------------------------------------------------------------
void ColorEngineRangeDefinitionDialog::setWidthRatio(const double width)
{
  if(m_widthSlider->value() != width*100)
  {
    m_widthSlider->setValue(width * 100);
  }
}

//--------------------------------------------------------------------
const double ColorEngineRangeDefinitionDialog::getWidthRatio() const
{
  return m_widthSlider->value()/100.;
}

//--------------------------------------------------------------------
void ColorEngineRangeDefinitionDialog::setHeightRatio(const double height)
{
  if(m_heightSlider->value() != height)
  {
    m_heightSlider->setValue(height * 100);
  }
}

//--------------------------------------------------------------------
const double ColorEngineRangeDefinitionDialog::getHeightRatio() const
{
  return m_heightSlider->value()/100.;
}

//--------------------------------------------------------------------
void ColorEngineRangeDefinitionDialog::setBarRatio(const double ratio)
{
  if(m_barRatioSlider->value() != ratio*100)
  {
    m_barRatioSlider->setValue(ratio * 100);
  }
}

//--------------------------------------------------------------------
const double ColorEngineRangeDefinitionDialog::getBarRatio() const
{
  return m_barRatioSlider->value()/100.;
}

//--------------------------------------------------------------------
void ColorEngineRangeDefinitionDialog::onOrientationChanged(int value)
{
  auto width = m_widthSlider->value();
  auto height = m_heightSlider->value();
  m_heightSlider->blockSignals(true);
  m_heightSlider->setValue(width);
  m_heightSlider->blockSignals(false);
  m_widthSlider->blockSignals(true);
  m_widthSlider->setValue(height);
  m_widthSlider->blockSignals(false);
}

//--------------------------------------------------------------------
void ColorEngineRangeDefinitionDialog::connectSignals()
{
  connect(m_showRange,         SIGNAL(stateChanged(int)),        this, SIGNAL(widgetsEnabled(int)));
  connect(m_showRange,         SIGNAL(stateChanged(int)),        this, SLOT(onWidgetsEnabled(int)));
  connect(m_positionCombo,     SIGNAL(currentIndexChanged(int)), this, SIGNAL(widgetsPropertiesModified()));
  connect(m_orientationCombo,  SIGNAL(currentIndexChanged(int)), this, SLOT(onOrientationChanged(int)), Qt::DirectConnection);
  connect(m_orientationCombo,  SIGNAL(currentIndexChanged(int)), this, SIGNAL(widgetsPropertiesModified()));
  connect(m_textPositionCombo, SIGNAL(currentIndexChanged(int)), this, SIGNAL(widgetsPropertiesModified()));
  connect(m_labelsSpin,        SIGNAL(valueChanged(int)),        this, SIGNAL(widgetsPropertiesModified()));
  connect(m_widthSlider,       SIGNAL(valueChanged(int)),        this, SIGNAL(widgetsPropertiesModified()));
  connect(m_heightSlider,      SIGNAL(valueChanged(int)),        this, SIGNAL(widgetsPropertiesModified()));
  connect(m_barRatioSlider,    SIGNAL(valueChanged(int)),        this, SIGNAL(widgetsPropertiesModified()));
  connect(m_decimalsSpin,      SIGNAL(valueChanged(int)),        this, SIGNAL(widgetsPropertiesModified()));
}

//--------------------------------------------------------------------
void ColorEngineRangeDefinitionDialog::setWidgetPosition(const Position position)
{
  if(position != getWidgetPosition())
  {
    m_positionCombo->setCurrentIndex(static_cast<int>(position));
  }
}

//--------------------------------------------------------------------
const ColorEngineRangeDefinitionDialog::Position ColorEngineRangeDefinitionDialog::getWidgetPosition() const
{
  return static_cast<Position>(m_positionCombo->currentIndex());
}

//--------------------------------------------------------------------
void ColorEngineRangeDefinitionDialog::setWidgetOrientation(const Orientation orientation)
{
  if(orientation != getWidgetOrientation())
  {
    m_orientationCombo->setCurrentIndex(static_cast<int>(orientation));
  }
}

//--------------------------------------------------------------------
const ColorEngineRangeDefinitionDialog::Orientation ColorEngineRangeDefinitionDialog::getWidgetOrientation() const
{
  return static_cast<Orientation>(m_orientationCombo->currentIndex());
}

//--------------------------------------------------------------------
void ColorEngineRangeDefinitionDialog::setTitlePosition(const TextPosition position)
{
  if(position != getTitlePosition())
  {
    m_textPositionCombo->setCurrentIndex(static_cast<int>(position));
  }
}

//--------------------------------------------------------------------
const ColorEngineRangeDefinitionDialog::TextPosition ColorEngineRangeDefinitionDialog::getTitlePosition() const
{
  return static_cast<TextPosition>(m_textPositionCombo->currentIndex());
}

//--------------------------------------------------------------------
void ColorEngineRangeDefinitionDialog::setNumberOfLabels(const int labelsNum)
{
  if(labelsNum != m_labelsSpin->value())
  {
    m_labelsSpin->setValue(labelsNum);
  }
}

//--------------------------------------------------------------------
const int ColorEngineRangeDefinitionDialog::getNumberOfLabels() const
{
  return m_labelsSpin->value();
}

//--------------------------------------------------------------------
void ColorEngineRangeDefinitionDialog::setNumberOfDecimals(const int decimals)
{
  if(decimals != m_decimalsSpin->value())
  {
    m_decimalsSpin->setValue(decimals);
  }
}

//--------------------------------------------------------------------
const int ColorEngineRangeDefinitionDialog::getNumberOfDecimals() const
{
  return m_decimalsSpin->value();
}
