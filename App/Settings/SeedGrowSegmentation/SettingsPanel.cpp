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

// EspINA
#include "SettingsPanel.h"
#include <Toolbars/Segmentation/SeedGrowSegmentationSettings.h>
#include <Core/EspinaSettings.h>
#include <GUI/ViewManager.h>

// Qt
#include <QSettings>
#include <QString>
#include <QDebug>

// VTK
#include <vtkMath.h>

using namespace EspINA;

const QString FIT_TO_SLICES ("ViewManager::FitToSlices");

//------------------------------------------------------------------------
SeedGrowSegmentationsSettingsPanel::SeedGrowSegmentationsSettingsPanel(SeedGrowSegmentationSettings *settings, ViewManager *viewManager)
: m_settings(settings)
, m_viewManager(viewManager)
, m_zValueChanged(false)
{
  setupUi(this);
  QSettings espinaSettings(CESVIMA, ESPINA);

  connect(m_pixelValue,SIGNAL(valueChanged(int)),
          this, SLOT(displayColor(int)));

  m_pixelValue->setValue(settings->bestPixelValue());
  displayColor(m_pixelValue->value());

  m_xSize->setValue(settings->xSize());
  m_ySize->setValue(settings->ySize());
  if (espinaSettings.value(FIT_TO_SLICES).toBool())
  {
    double zSpacing = 1.0;
    if (m_viewManager->viewResolution() != NULL)
      zSpacing = m_viewManager->viewResolution()[2];

    m_zSize->setValue(vtkMath::Round(settings->zSize()/zSpacing));
    m_zSize->setSuffix(QString(" slices"));
  }
  else
  {
    m_zSize->setValue(settings->zSize());
    m_zSize->setSuffix(QString(" nm"));
  }

  m_taxonomicalVOI->setChecked(settings->taxonomicalVOI());
  m_xSize->setEnabled(!settings->taxonomicalVOI());
  m_ySize->setEnabled(!settings->taxonomicalVOI());
  m_zSize->setEnabled(!settings->taxonomicalVOI());

  connect(m_taxonomicalVOI, SIGNAL(stateChanged(int)),
          this, SLOT(changeTaxonomicalCheck(int)));

  // the rounding of fit to slices on the z value was making the dialog ask the
  // user if he wanted to save the changes even when the user hasn't changed
  // anything. this fixes it.
  connect(m_zSize, SIGNAL(valueChanged(int)), this, SLOT(zValueChanged(int)));

  bool closingActive = settings->closing() > 0;
  m_applyClosing->setChecked(closingActive);
  m_closing->setEnabled(closingActive);
  m_closing->setValue(settings->closing());

  connect(m_applyClosing, SIGNAL(toggled(bool)),
          m_closing, SLOT(setEnabled(bool)));
}

//------------------------------------------------------------------------
void SeedGrowSegmentationsSettingsPanel::acceptChanges()
{
  m_settings->setBestPixelValue(m_pixelValue->value());

  if (!m_taxonomicalVOI->isChecked())
  {
    m_settings->setXSize(m_xSize->value());
    m_settings->setYSize(m_ySize->value());

    double zSpacing = 1.0;
    if (m_viewManager->viewResolution() != NULL)
      zSpacing = m_viewManager->viewResolution()[2];

    m_settings->setZSize(m_zSize->value()*zSpacing);
  }
  m_settings->setTaxonomicalVOI(m_taxonomicalVOI->isChecked());

  if (m_applyClosing->isChecked())
    m_settings->setClosing(m_closing->value());
  else
    m_settings->setClosing(0);
}

//------------------------------------------------------------------------
void SeedGrowSegmentationsSettingsPanel::rejectChanges()
{
}

//------------------------------------------------------------------------
bool SeedGrowSegmentationsSettingsPanel::modified() const
{
  QSettings settings(CESVIMA, ESPINA);

  bool returnValue = false;
  returnValue |= (m_xSize->value() != m_settings->xSize());
  returnValue |= (m_ySize->value() != m_settings->ySize());
  returnValue |= m_zValueChanged;
  returnValue |= (m_taxonomicalVOI->isChecked() != m_settings->taxonomicalVOI());
  returnValue |= (m_pixelValue->value() != m_settings->bestPixelValue());
  returnValue |= ((m_applyClosing->isChecked() ? m_closing->value() : 0) != m_settings->closing());

  return returnValue;
}

//------------------------------------------------------------------------
ISettingsPanelPtr SeedGrowSegmentationsSettingsPanel::clone()
{
  return ISettingsPanelPtr(new SeedGrowSegmentationsSettingsPanel(m_settings, m_viewManager));
}

//------------------------------------------------------------------------
void SeedGrowSegmentationsSettingsPanel::displayColor(int value)
{
  QPixmap pic(32,32);
  pic.fill(QColor(value,value,value));
  m_colorSample->setPixmap(pic);
  m_colorSample->setToolTip(tr("Pixel Value: %1").arg(value));
  m_pixelValue->setToolTip(tr("Pixel Value: %1").arg(value));
}

//------------------------------------------------------------------------
void SeedGrowSegmentationsSettingsPanel::changeTaxonomicalCheck(int state)
{
  bool value = (Qt::Checked != state);
  m_xSize->setEnabled(value);
  m_ySize->setEnabled(value);
  m_zSize->setEnabled(value);
}

//------------------------------------------------------------------------
void SeedGrowSegmentationsSettingsPanel::zValueChanged(int unused)
{
  m_zValueChanged = true;
}
