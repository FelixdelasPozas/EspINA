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

//------------------------------------------------------------------------
ThresholdAction::ThresholdAction(QObject* parent)
: QWidgetAction(parent)
{
  QSettings settings("CeSViMa", "EspIA");
  m_threshold = settings.value("SeedGrowSegmentation::Threshold", DEFAULT_THRESHOLD).toInt();
}

//------------------------------------------------------------------------
QWidget* ThresholdAction::createWidget(QWidget* parent)
{
  QWidget *w = new QWidget(parent);
  QHBoxLayout *layout = new QHBoxLayout();
  w->setLayout(layout);

  // Threshold Widget
  QLabel *thresholdLabel = new QLabel(tr("Threshold"));
  QSpinBox *threshold = new QSpinBox();
  threshold->setMinimum(0);
  threshold->setMaximum(255);
  threshold->setValue(m_threshold);
  threshold->setToolTip(tr("Determine the size of color value range for a given pixel"));

  layout->addWidget(thresholdLabel);
  layout->addWidget(threshold);

  connect(threshold,SIGNAL(valueChanged(int)),
	  this, SLOT(setThreshold(int)));
  connect(this, SIGNAL(thresholdChanged(int)),
	  threshold, SLOT(setValue(int)));

  return w;
}

void ThresholdAction::setThreshold(int th)
{
  m_threshold = th>0?th:0;
  QSettings settings("CeSViMa", "EspIA");
  settings.setValue("SeedGrowSegmentation::Threshold", m_threshold);
  emit thresholdChanged(m_threshold);
}