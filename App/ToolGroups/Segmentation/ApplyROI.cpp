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


#include "ApplyROI.h"

#include <QCheckBox>
#include <QHBoxLayout>

using namespace EspINA;

//------------------------------------------------------------------------
ApplyROI::ApplyROI(QObject* parent)
: QWidgetAction(parent)
, m_useROI(true)
{
  for(int i = 0; i < 3; ++i)
  {
    m_labelROI[i]   = nullptr;
    m_spinBoxROI[i] = nullptr;
  }
}


//------------------------------------------------------------------------
QWidget* ApplyROI::createWidget(QWidget* parent)
{
  //if (m_currentWidget) delete m_currentWidget;

  QWidget *widget = new QWidget(parent);

  QCheckBox *roiCheckBox = new QCheckBox(tr("Apply ROI"), parent);
  roiCheckBox->setCheckState(Qt::Checked);

  QString labels[3] = {"X:", "Y:", "Z:"};
  for(int i = 0; i < 3; ++i)
  {
    m_labelROI[i] = new QLabel(widget);
    m_labelROI[i]->setText(labels[i]);
    m_labelROI[i]->setVisible(m_useROI);

    m_spinBoxROI[i] = new QSpinBox(widget);
    m_spinBoxROI[i]->setVisible(m_useROI);
    m_spinBoxROI[i]->setMinimum(0);
    m_spinBoxROI[i]->setMaximum(1000);
  }

  QHBoxLayout *mainLaout = new QHBoxLayout(widget);
  mainLaout->addWidget(roiCheckBox);
  for (int i = 0; i < 3; ++i)
  {
    mainLaout->addWidget(m_labelROI[i]);
    mainLaout->addWidget(m_spinBoxROI[i]);
  }

  widget->setLayout(mainLaout);

  connect(roiCheckBox, SIGNAL(toggled(bool)),
          this, SLOT(onValueChanged(bool)));

  return widget;
}

//------------------------------------------------------------------------
void ApplyROI::onValueChanged(bool value)
{
   m_useROI = value;

  for(int i = 0; i < 3; ++i)
  {
    if (m_labelROI[i] && m_spinBoxROI[i])
    {
      m_labelROI  [i]->setVisible(m_useROI);
      m_spinBoxROI[i]->setVisible(m_useROI);
    }
  }

   emit useROI(value);
}
