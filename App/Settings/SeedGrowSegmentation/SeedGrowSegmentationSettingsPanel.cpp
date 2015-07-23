/*

    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>

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
#include "SeedGrowSegmentationSettingsPanel.h"

#include <Support/Settings/EspinaSettings.h>
#include <GUI/Widgets/PixelValueSelector.h>
#include <ToolGroups/Segment/SeedGrowSegmentation/SeedGrowSegmentationSettings.h>

// Qt
#include <QSettings>
#include <QString>
#include <QDebug>

// VTK
#include <vtkMath.h>

using namespace ESPINA;
using namespace ESPINA::GUI::Widgets;
using namespace ESPINA::Support::Settings;

//------------------------------------------------------------------------
SeedGrowSegmentationsSettingsPanel::SeedGrowSegmentationsSettingsPanel(SeedGrowSegmentationSettings *settings)
: m_settings     {settings}
, m_pixelSelector{new PixelValueSelector(this)}
{
  setupUi(this);

  m_pixelSelector->setFixedHeight(24);

  m_colorFrame->setLayout(new QHBoxLayout());
  m_colorFrame->layout()->setMargin(0);
  m_colorFrame->layout()->addWidget(m_pixelSelector);

  m_pixelSelector->setValue(settings->bestPixelValue());
  m_xSize->setValue(settings->xSize());
  m_ySize->setValue(settings->ySize());
  m_zSize->setValue(settings->zSize());
  m_applyCategoryROI->setChecked(settings->applyCategoryROI());
  m_xSize->setEnabled(!settings->applyCategoryROI());
  m_ySize->setEnabled(!settings->applyCategoryROI());
  m_zSize->setEnabled(!settings->applyCategoryROI());

  m_applyClosing->setEnabled(settings->applyClose());
  m_closing->setValue(settings->closeRadius());

  connect(m_applyCategoryROI, SIGNAL(stateChanged(int)),
          this,               SLOT(changeTaxonomicalCheck(int)));

  connect(m_applyClosing, SIGNAL(toggled(bool)),
          m_closing, SLOT(setEnabled(bool)));
}

//------------------------------------------------------------------------
void SeedGrowSegmentationsSettingsPanel::acceptChanges()
{
  m_settings->setBestPixelValue(m_pixelSelector->value());

  if (!m_applyCategoryROI->isChecked())
  {
    m_settings->setXSize(m_xSize->value());
    m_settings->setYSize(m_ySize->value());
    m_settings->setZSize(m_zSize->value());
  }

  m_settings->setApplyCategoryROI(m_applyCategoryROI->isChecked());

  if (m_applyClosing->isChecked())
  {
    m_settings->setCloseRadius(m_closing->value());
  }
  else
  {
    m_settings->setCloseRadius(0);
  }
}

//------------------------------------------------------------------------
void SeedGrowSegmentationsSettingsPanel::rejectChanges()
{
}

//------------------------------------------------------------------------
bool SeedGrowSegmentationsSettingsPanel::modified() const
{
  ESPINA_SETTINGS(settings);

  bool returnValue = false;
  returnValue |= (m_xSize->value() != m_settings->xSize());
  returnValue |= (m_ySize->value() != m_settings->ySize());
  returnValue |= (m_zSize->value() != m_settings->zSize());
  returnValue |= (m_applyCategoryROI->isChecked() != m_settings->applyCategoryROI());
  returnValue |= (m_pixelSelector->value()        != m_settings->bestPixelValue());
  returnValue |= (m_applyClosing->isChecked()     != m_settings->applyClose());
  returnValue |= (m_applyClosing->isChecked() ? returnValue : (m_closing->value() != m_settings->closeRadius()));

  return returnValue;
}

//------------------------------------------------------------------------
SettingsPanelPtr SeedGrowSegmentationsSettingsPanel::clone()
{
  return new SeedGrowSegmentationsSettingsPanel(m_settings);
}

//------------------------------------------------------------------------
void SeedGrowSegmentationsSettingsPanel::changeTaxonomicalCheck(int state)
{
  bool value = (Qt::Checked != state);
  m_xSize->setEnabled(value);
  m_ySize->setEnabled(value);
  m_zSize->setEnabled(value);
}
