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
#include "RawInformationDialog.h"
#include <GUI/Dialogs/DefaultDialogs.h>
#include <Support/Settings/Settings.h>
#include <Support/Widgets/TabularReport.h>
#include <QSettings>
#include <QDialogButtonBox>
#include <QLayout>

using ESPINA::GUI::DefaultDialogs;

using namespace ESPINA;

const QString SETTINGS_GROUP = "Raw Information Report";

//----------------------------------------------------------------------------
RawInformationDialog::RawInformationDialog(SegmentationAdapterList input, Support::Context &context)
: QDialog(DefaultDialogs::defaultParentWidget(), Qt::WindowFlags{Qt::WindowMinMaxButtonsHint|Qt::WindowCloseButtonHint})
{
  setObjectName("Raw Information Report");

  setWindowTitle(tr("Raw Information Report"));

  auto report = new TabularReport(context, this);
  report->setFilter(input);
  report->setModel(context.model());

  setLayout(new QVBoxLayout());
  layout()->addWidget(report);

  auto acceptButton = new QDialogButtonBox(QDialogButtonBox::Ok);
  connect(acceptButton, SIGNAL(accepted()),
          this,         SLOT(accept()));
  layout()->addWidget(acceptButton);

  ESPINA_SETTINGS(settings);

  auto pos = DefaultDialogs::defaultParentWidget()->pos();
  pos.setX(pos.x()+200);
  pos.setY(pos.y()+200);

  settings.beginGroup(SETTINGS_GROUP);
  resize(settings.value("size", QSize(450, 250)).toSize());
  move  (settings.value("pos",  pos).toPoint());
  settings.endGroup();
}

//----------------------------------------------------------------------------
void RawInformationDialog::closeEvent(QCloseEvent *event)
{
  ESPINA_SETTINGS(settings);

  settings.beginGroup(SETTINGS_GROUP);
  settings.setValue("size", size());
  settings.setValue("pos", pos());
  settings.endGroup();

  QDialog::closeEvent(event);
}
