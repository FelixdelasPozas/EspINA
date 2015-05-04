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
#include "SeedThreshold.h"
#include <Support/Settings/EspinaSettings.h>

// Qt
#include <QHBoxLayout>
#include <QLabel>
#include <QSpinBox>
#include <QSettings>

const int DEFAULT_THRESHOLD = 30;
const int MIN_THRESHOLD = 0;
const int MAX_THRESHOLD = 255;

const QString LTHRESHOLD = "SeedGrowSegmentation::LowerThreshold";
const QString UTHRESHOLD = "SeedGrowSegmentation::UpperThreshold";

using namespace ESPINA;

//------------------------------------------------------------------------
SeedThreshold::SeedThreshold(QWidget* parent)
: QWidget      {parent}
, m_lthLabel   {nullptr}
, m_uthLabel   {nullptr}
, m_lth        {nullptr}
, m_uth        {nullptr}
, m_symmetrical{true}
{
  ESPINA_SETTINGS(settings);
  m_threshold[0] = settings.value(LTHRESHOLD, DEFAULT_THRESHOLD).toInt();
  m_threshold[1] = settings.value(UTHRESHOLD, DEFAULT_THRESHOLD).toInt();

  // Lower Threshold Widget
  m_lthLabel = new QLabel(tr("Lower Th."));
  m_lth      = new QSpinBox();

  m_lth->setMinimum(0);
  m_lth->setMaximum(255);
  m_lth->setValue(m_threshold[0]);
  m_lth->setToolTip(tr("Determine the size of color value range for a given pixel"));

  connect(m_lth,SIGNAL(valueChanged(int)),
          this, SLOT(setLowerThreshold(int)));
  connect(this, SIGNAL(lowerThresholdChanged(int)),
          m_lth, SLOT(setValue(int)));

  // Upper Threshold Widget
  m_uthLabel = new QLabel(tr("Upper Th."));
  m_uth      = new QSpinBox();

  m_uth->setMinimum(0);
  m_uth->setMaximum(255);
  m_uth->setValue(m_threshold[1]);
  m_uth->setToolTip(tr("Determine the size of color value range for a given pixel"));

  setSymmetricalThreshold(true);

  connect(m_uth,SIGNAL(valueChanged(int)),
          this, SLOT(setUpperThreshold(int)));
  connect(this, SIGNAL(upperThresholdChanged(int)),
          m_uth, SLOT(setValue(int)));

  auto layout = new QHBoxLayout();

  layout->addWidget(m_lthLabel);
  layout->addWidget(m_lth);
  layout->addWidget(m_uthLabel);
  layout->addWidget(m_uth);

  setLayout(layout);
}

//-----------------------------------------------------------------------------
void SeedThreshold::setSymmetricalThreshold(bool symmetrical)
{
  m_uthLabel->setVisible(!symmetrical);
  m_uth->setVisible(!symmetrical);

  m_lthLabel->setText(symmetrical?tr("Threshold"):tr("Lower Th."));
  m_symmetrical = symmetrical;
}

//-----------------------------------------------------------------------------
void SeedThreshold::setLowerThreshold(int th)
{
  m_threshold[0] = validThreshold(th);

  ESPINA_SETTINGS(settings);
  settings.setValue(LTHRESHOLD, m_threshold[0]);

  emit lowerThresholdChanged(m_threshold[0]);

  if (m_symmetrical)
    setUpperThreshold(th);
}

//-----------------------------------------------------------------------------
void SeedThreshold::setUpperThreshold(int th)
{
  m_threshold[1] = validThreshold(th);

  ESPINA_SETTINGS(settings);
  settings.setValue(UTHRESHOLD, m_threshold[1]);

  emit upperThresholdChanged(m_threshold[1]);
}

//-----------------------------------------------------------------------------
int SeedThreshold::validThreshold(int value) const
{
  return validRangedValue(value, MIN_THRESHOLD, MAX_THRESHOLD);
}

//-----------------------------------------------------------------------------
int SeedThreshold::validRangedValue(int value, int min, int max) const
{
  return std::max(min, std::min(value, max));
}

