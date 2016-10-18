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

#include "DistanceInformationDialog.h"
#include <GUI/Dialogs/DefaultDialogs.h>
#include <Support/Settings/Settings.h>
#include <Support/Widgets/TabularReport.h>

using ESPINA::GUI::DefaultDialogs;
using namespace ESPINA;

const QString SETTINGS_GROUP = "Raw Information Report";

//----------------------------------------------------------------------------
DistanceInformationDialog::DistanceInformationDialog(SegmentationAdapterList input, DistOpts options, Support::Context &context)
: QDialog(DefaultDialogs::defaultParentWidget(), Qt::WindowStaysOnTopHint)
, input(input)
, context(context)
, options(options)
{
  setWindowTitle(tr("Distance Information Report"));
  setWindowIconText(":/espina/Espina.svg");
  setLayout(new QVBoxLayout());

  auto report = new TabularReport(context, this);
  report->setModel(context.model());
  report->setFilter(input);

  layout()->addWidget(report);
  auto acceptButton = new QDialogButtonBox(QDialogButtonBox::Ok);
  connect(acceptButton, SIGNAL(accepted()),
          this,         SLOT(accept()));
  layout()->addWidget(acceptButton);
}

