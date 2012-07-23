/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  Jorge Peña Pastor <jpena@cesvima.upm.es>

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


#include "ThresholdAction.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QSpinBox>

#include <QDebug>
#include <QSettings>

#define DEFAULT_THRESHOLD 30

const QString LTHRESHOLD = "SeedGrowSegmentation::LowerThreshold";
const QString UTHRESHOLD = "SeedGrowSegmentation::UpperThreshold";

//------------------------------------------------------------------------
ThresholdAction::ThresholdAction(QObject* parent)
: QWidgetAction(parent)
{
  QSettings settings("CeSViMa", "EspINA");
  m_threshold[0] = settings.value(LTHRESHOLD, DEFAULT_THRESHOLD).toInt();
  m_threshold[1] = settings.value(UTHRESHOLD, DEFAULT_THRESHOLD).toInt();
}

//------------------------------------------------------------------------
QWidget* ThresholdAction::createWidget(QWidget* parent)
{
  QWidget *w = new QWidget(parent);
  QHBoxLayout *layout = new QHBoxLayout();
  w->setLayout(layout);

  // Lower Threshold Widget
  m_lthLabel = new QLabel(tr("Lower Th."));
  m_lth = new QSpinBox();
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
  m_uth = new QSpinBox();
  m_uth->setMinimum(0);
  m_uth->setMaximum(255);
  m_uth->setValue(m_threshold[1]);
  m_uth->setToolTip(tr("Determine the size of color value range for a given pixel"));

  connect(m_uth,SIGNAL(valueChanged(int)),
          this, SLOT(setUpperThreshold(int)));
  connect(this, SIGNAL(upperThresholdChanged(int)),
          m_uth, SLOT(setValue(int)));

  layout->addWidget(m_lthLabel);
  layout->addWidget(m_lth);
  layout->addWidget(m_uthLabel);
  layout->addWidget(m_uth);

  return w;
}

//-----------------------------------------------------------------------------
void ThresholdAction::setSymmetricalThreshold(bool symmetrical)
{
  m_uthLabel->setVisible(!symmetrical);
  m_uth->setVisible(!symmetrical);

  m_lthLabel->setText(symmetrical?tr("Threshold"):tr("Lower Th."));
}


//-----------------------------------------------------------------------------
void ThresholdAction::setLowerThreshold(int th)
{
  m_threshold[0] = th>0?th:0;
  QSettings settings("CeSViMa", "EspINA");
  settings.setValue(LTHRESHOLD, m_threshold[0]);
  emit lowerThresholdChanged(m_threshold[0]);

  if (m_symmetrical)
    setUpperThreshold(th);
}

//-----------------------------------------------------------------------------
void ThresholdAction::setUpperThreshold(int th)
{
  m_threshold[1] = th>0?th:0;
  QSettings settings("CeSViMa", "EspINA");
  settings.setValue(UTHRESHOLD, m_threshold[1]);
  emit upperThresholdChanged(m_threshold[1]);
}

