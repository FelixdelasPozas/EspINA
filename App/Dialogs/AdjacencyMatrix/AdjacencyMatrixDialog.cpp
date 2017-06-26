/*

 Copyright (C) 2017 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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
#include <Dialogs/AdjacencyMatrix/AdjacencyMatrixDialog.h>
#include <Dialogs/AdjacencyMatrix/AdjacencyMatrixTabularReport.h>
#include <GUI/Dialogs/DefaultDialogs.h>
#include <Support/Settings/Settings.h>

// Qt
#include <QCloseEvent>
#include <QDialogButtonBox>

using namespace ESPINA;
using namespace ESPINA::GUI;
using namespace ESPINA::Support;

const QString SETTINGS_GROUP = "Adjacency Matrix";

//--------------------------------------------------------------------
AdjacencyMatrixDialog::AdjacencyMatrixDialog(SegmentationAdapterList segmentations, Support::Context &context)
: QDialog  {DefaultDialogs::defaultParentWidget(), Qt::WindowFlags{Qt::WindowMinMaxButtonsHint|Qt::WindowCloseButtonHint}}
{
  setWindowTitle(tr("Adjacency Matrix Report"));
  setWindowIconText(":/espina/espina.svg");
  setLayout(new QVBoxLayout());

  auto report = new AdjacencyMatrixTabularReport(segmentations, context);

  layout()->addWidget(report);

  auto acceptButton = new QDialogButtonBox(QDialogButtonBox::Ok);

  connect(acceptButton, SIGNAL(accepted()),
          this,         SLOT(close()));

  layout()->addWidget(acceptButton);

  window()->resize(layout()->sizeHint());
  window()->adjustSize();

  ESPINA_SETTINGS(settings);

  settings.beginGroup(SETTINGS_GROUP);
  resize(settings.value("size", QSize (400, 200)).toSize());
  move  (settings.value("pos",  QPoint(200, 200)).toPoint());
  settings.endGroup();}

//--------------------------------------------------------------------
void AdjacencyMatrixDialog::closeEvent(QCloseEvent *event)
{
  ESPINA_SETTINGS(settings);

  settings.beginGroup(SETTINGS_GROUP);
  settings.setValue("size", size());
  settings.setValue("pos", pos());
  settings.endGroup();

  QDialog::closeEvent(event);
}
