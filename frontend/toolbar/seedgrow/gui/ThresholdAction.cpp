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
  QLabel *lthresholdLabel = new QLabel(tr("Lower Th."));
  QSpinBox *lthreshold = new QSpinBox();
  lthreshold->setMinimum(0);
  lthreshold->setMaximum(255);
  lthreshold->setValue(m_threshold[0]);
  lthreshold->setToolTip(tr("Determine the size of color value range for a given pixel"));

  connect(lthreshold,SIGNAL(valueChanged(int)),
          this, SLOT(setLowerThreshold(int)));
  connect(this, SIGNAL(lowerThresholdChanged(int)),
          lthreshold, SLOT(setValue(int)));

  // Upper Threshold Widget
  QLabel *uthresholdLabel = new QLabel(tr("Upper Th."));
  QSpinBox *uthreshold = new QSpinBox();
  uthreshold->setMinimum(0);
  uthreshold->setMaximum(255);
  uthreshold->setValue(m_threshold[1]);
  uthreshold->setToolTip(tr("Determine the size of color value range for a given pixel"));

  connect(uthreshold,SIGNAL(valueChanged(int)),
          this, SLOT(setUpperThreshold(int)));
  connect(this, SIGNAL(upperThresholdChanged(int)),
          uthreshold, SLOT(setValue(int)));

  layout->addWidget(lthresholdLabel);
  layout->addWidget(lthreshold);
  layout->addWidget(uthresholdLabel);
  layout->addWidget(uthreshold);


  return w;
}

void ThresholdAction::setLowerThreshold(int th)
{
  m_threshold[0] = th>0?th:0;
  QSettings settings("CeSViMa", "EspINA");
  settings.setValue(LTHRESHOLD, m_threshold[0]);
  emit lowerThresholdChanged(m_threshold[0]);
}

void ThresholdAction::setUpperThreshold(int th)
{
  m_threshold[1] = th>0?th:0;
  QSettings settings("CeSViMa", "EspINA");
  settings.setValue(UTHRESHOLD, m_threshold[1]);
  emit upperThresholdChanged(m_threshold[1]);
}

