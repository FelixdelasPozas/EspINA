/*
 * Copyright (C) 2016, Rafael Juan Vicente Garcia <rafaelj.vicente@gmail.com>
 *
 * This file is part of ESPINA.
 *
 * ESPINA is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

// ESPINA
#include "DistanceInformationDialog.h"
#include "DistanceInformationTabularReport.h"
#include <Support/Settings/Settings.h>

// Qt
#include <QPushButton>
#include <QSettings>
#include <QHBoxLayout>
#include <QDialogButtonBox>

using namespace ESPINA;
using namespace ESPINA::GUI;

const QString SETTINGS_GROUP = "Distance Information Report";

//----------------------------------------------------------------------------
DistanceInformationDialog::DistanceInformationDialog(const SegmentationAdapterList segmentations,
                                                     const DistanceInformationDialog::DistancesMap distances,
                                                     const DistanceInformationOptionsDialog::Options &options,
                                                     Support::Context &context)
: QDialog(DefaultDialogs::defaultParentWidget())
{
  setWindowTitle(tr("Distance Information Report"));
  setWindowIconText(":/espina/Espina.svg");
  setLayout(new QVBoxLayout());

  auto report = new DistanceInformationTabularReport(context, segmentations, options, distances);

  layout()->addWidget(report);

  auto acceptButton = new QDialogButtonBox(QDialogButtonBox::Ok);

  connect(acceptButton, SIGNAL(accepted()),
          this,         SLOT(accept()));

  layout()->addWidget(acceptButton);

  window()->resize(layout()->sizeHint());
  window()->adjustSize();

  ESPINA_SETTINGS(settings);

  settings.beginGroup(SETTINGS_GROUP);
  resize(settings.value("size", QSize (400, 200)).toSize());
  move  (settings.value("pos",  QPoint(200, 200)).toPoint());
  settings.endGroup();
}

//----------------------------------------------------------------------------
void DistanceInformationDialog::closeEvent(QCloseEvent *event)
{
  ESPINA_SETTINGS(settings);

  settings.beginGroup(SETTINGS_GROUP);
  settings.setValue("size", size());
  settings.setValue("pos", pos());
  settings.endGroup();

  QDialog::closeEvent(event);
}

