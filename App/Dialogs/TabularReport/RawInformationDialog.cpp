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


#include "RawInformationDialog.h"

#include "TabularReport.h"
#include <Core/Model/Proxies/TaxonomyProxy.h>
#include <Core/EspinaSettings.h>
#include <QSettings>
#include <QDialogButtonBox>

#ifdef TEST_ESPINA_MODELS
#include <Core/Model/ModelTest.h>
#endif

using namespace EspINA;

//----------------------------------------------------------------------------
RawInformationDialog::RawInformationDialog(EspinaModel *model,
                             ViewManager *viewManager,
                             QWidget     *parent)
: QDialog(parent)
{
  setObjectName("Raw Information Analysis");

  setWindowTitle(tr("Raw Information"));

  TabularReport *report = new TabularReport(viewManager, this);
  report->setModel(model);
  setLayout(new QVBoxLayout());
  layout()->addWidget(report);

  QDialogButtonBox *acceptButton = new QDialogButtonBox(QDialogButtonBox::Ok);
  connect(acceptButton, SIGNAL(accepted()),
          this,         SLOT(accept()));
  layout()->addWidget(acceptButton);

  QSettings settings(CESVIMA, ESPINA);

  settings.beginGroup("Raw Information Analysis");
  resize(settings.value("size", QSize(200, 200)).toSize());
  move(settings.value("pos", QPoint(200, 200)).toPoint());
  settings.endGroup();
}

//----------------------------------------------------------------------------
RawInformationDialog::~RawInformationDialog()
{

}

//----------------------------------------------------------------------------
void RawInformationDialog::closeEvent(QCloseEvent *event)
{
  QSettings settings(CESVIMA, ESPINA);

  settings.beginGroup("Raw Information Analysis");
  settings.setValue("size", size());
  settings.setValue("pos", pos());
  settings.endGroup();

  QDialog::closeEvent(event);
}
