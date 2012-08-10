/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  Jorge PeÃ±a Pastor <email>

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


#include "CODESettings.h"

#include "MorphologicalEditionFilter.h"
#include <EspinaCore.h>
#include <EspinaView.h>
#include <QSpinBox>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QApplication>

//----------------------------------------------------------------------------
CODESettings::CODESettings(MorphologicalEditionFilter *filter)
: QWidget()
, m_filter(filter)
{
  QGroupBox *group = new QGroupBox(tr("Morphological Edition Tool"), this);
  QLabel *label = new QLabel(tr("Radius"));
  m_spinbox = new QSpinBox();
  m_spinbox->setMinimum(0);
  m_spinbox->setMaximum(100);
  m_spinbox->setValue(filter->radius());

  QHBoxLayout *groupLayout = new QHBoxLayout();
  groupLayout->addWidget(label);
  groupLayout->addWidget(m_spinbox);
  group->setLayout(groupLayout);

  QPushButton *modifyButton = new QPushButton(tr("Modify"));
  QHBoxLayout *modifyLayout = new QHBoxLayout();
  modifyLayout->addStretch();
  modifyLayout->addWidget(modifyButton);
  modifyLayout->addStretch();

  QVBoxLayout *mainLayout = new QVBoxLayout();
  mainLayout->addWidget(group);
  mainLayout->addLayout(modifyLayout);
  mainLayout->addStretch();
  setLayout(mainLayout);

  connect(modifyButton, SIGNAL(clicked(bool)),
          this, SLOT(modifyFilter()));
}

//----------------------------------------------------------------------------
CODESettings::~CODESettings()
{

}

//----------------------------------------------------------------------------
void CODESettings::modifyFilter()
{
  m_filter->setRadius(m_spinbox->value());
  QApplication::setOverrideCursor(Qt::WaitCursor);
  m_filter->update();
  EspinaCore::instance()->viewManger()->currentView()->forceRender();
  QApplication::restoreOverrideCursor();
}