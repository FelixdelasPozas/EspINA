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
#include <Support/Utils/SelectionUtils.h>
#include <Support/Widgets/TabularReport.h>
#include <Support/Settings/EspinaSettings.h>

// Qt
#include <QSettings>
#include <QDialogButtonBox>
#include <QLayout>

using namespace ESPINA;

//----------------------------------------------------------------------------
RawInformationDialog::RawInformationDialog(const Support::Context &context)

{
  setObjectName("Raw Information Analysis");

  setWindowTitle(tr("Raw Information"));

  auto report = new TabularReport(context, this);
  report->setModel(context.model());

  auto segmentations = defaultReportInputSegmentations(context.viewState(), context.model());
  report->setFilter(segmentations);

  setLayout(new QVBoxLayout());
  layout()->addWidget(report);

  QDialogButtonBox *acceptButton = new QDialogButtonBox(QDialogButtonBox::Ok);
  connect(acceptButton, SIGNAL(accepted()),
          this,         SLOT(accept()));
  layout()->addWidget(acceptButton);

  ESPINA_SETTINGS(settings);

  settings.beginGroup("Raw Information Analysis");
  resize(settings.value("size", QSize (200, 200)).toSize());
  move  (settings.value("pos",  QPoint(200, 200)).toPoint());
  settings.endGroup();
}

//----------------------------------------------------------------------------
void RawInformationDialog::closeEvent(QCloseEvent *event)
{
  ESPINA_SETTINGS(settings);

  settings.beginGroup("Raw Information Analysis");
  settings.setValue("size", size());
  settings.setValue("pos", pos());
  settings.endGroup();

  QDialog::closeEvent(event);
}
