/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2011  Jorge Peña <jorge.pena.pastor@gmail.com>

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

#include "SettingsPanel.h"

#include <Toolbars/SeedGrowSegmentation/SeedGrowSegmentationSettings.h>

#include <QSettings>
#include <QString>
#include <QDebug>

using namespace EspINA;

//------------------------------------------------------------------------
SeedGrowSegmentation::SettingsPanel::SettingsPanel(SeedGrowSegmentationSettings *settings)
: m_settings(settings)
{
  setupUi(this);

  connect(m_pixelValue,SIGNAL(valueChanged(int)),
          this, SLOT(displayColor(int)));

  m_pixelValue->setValue(settings->bestPixelValue());
  displayColor(m_pixelValue->value());

  m_xSize->setValue(settings->xSize());
  m_ySize->setValue(settings->ySize());
  m_zSize->setValue(settings->zSize());
  m_taxonomicalVOI->setChecked(settings->taxonomicalVOI());

  bool closingActive = settings->closing()>0;
  m_applyClosing->setChecked(closingActive);
  m_closing->setEnabled(closingActive);
  m_closing->setValue(settings->closing());

  connect(m_applyClosing, SIGNAL(toggled(bool)),
          m_closing, SLOT(setEnabled(bool)));
}

//------------------------------------------------------------------------
void SeedGrowSegmentation::SettingsPanel::acceptChanges()
{
  m_settings->setBestPixelValue(m_pixelValue->value());

  m_settings->setXSize(m_xSize->value());
  m_settings->setYSize(m_ySize->value());
  m_settings->setZSize(m_zSize->value());
  m_settings->setTaxonomicalVOI(m_taxonomicalVOI->isChecked());

  if (m_applyClosing->isChecked())
    m_settings->setClosing(m_closing->value());
  else
    m_settings->setClosing(0);
}

//------------------------------------------------------------------------
bool SeedGrowSegmentation::SettingsPanel::modified() const
{
  return m_xSize->value() != m_settings->xSize()
      || m_ySize->value() != m_settings->ySize()
      || m_zSize->value() != m_settings->zSize()
      || m_taxonomicalVOI->isChecked() != m_settings->taxonomicalVOI()
      || m_pixelValue->value() != m_settings->bestPixelValue()
      || (m_applyClosing->isChecked()?m_closing->value():0) != m_settings->closing();
}


//------------------------------------------------------------------------
ISettingsPanelPtr SeedGrowSegmentation::SettingsPanel::clone()
{
  return ISettingsPanelPtr(new SettingsPanel(m_settings));
}

//------------------------------------------------------------------------
void SeedGrowSegmentation::SettingsPanel::displayColor(int value)
{
  QPixmap pic(32,32);
  pic.fill(QColor(value,value,value));
  m_colorSample->setPixmap(pic);
  m_colorSample->setToolTip(tr("Pixel Value: %1").arg(value));
  m_pixelValue->setToolTip(tr("Pixel Value: %1").arg(value));
}
