/*
 * 
 * Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 * This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
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

// Plugin
#include "SASAnalysisDialog.h"
#include "SASTabularReport.h"

// ESPINA
#include <Core/Extensions/AppositionSurfaceExtension.h>
#include <GUI/Model/SegmentationAdapter.h>
#include <Core/Factory/CoreFactory.h>
#include <Support/Settings/EspinaSettings.h>
#include <Extensions/Morphological/MorphologicalInformation.h>
#include <GUI/Widgets/InformationSelector.h>
#include <App/Dialogs/TabularReport/TabularReport.h>

// Qt
#include <QPushButton>
#include <QFileDialog>
#include <QSettings>
#include <QHBoxLayout>
#include <QDialogButtonBox>

using namespace ESPINA;
using namespace xlslib_core;

//----------------------------------------------------------------------------
SASAnalysisDialog::SASAnalysisDialog(SegmentationAdapterList segmentations,
                                     ModelAdapterSPtr        model,
                                     QUndoStack             *undoStack,
                                     ModelFactorySPtr        factory,
                                     ViewManagerSPtr         viewManager,
                                     QWidget                *parent)
: QDialog(parent)
{
  setObjectName("Synaptic Apposition Surfaces Analysis");
  setWindowTitle(tr("Synaptic Apposition Surfaces Analysis"));
  setWindowIcon(QIcon(":/AppSurface.svg"));

  SASTabularReport *report = new SASTabularReport(model, factory, viewManager, this);
  report->setModel(model);
  report->setFilter(segmentations);

  setLayout(new QVBoxLayout());
  layout()->addWidget(report);

  QDialogButtonBox *acceptButton = new QDialogButtonBox(QDialogButtonBox::Ok);
  connect(acceptButton, SIGNAL(accepted()),
          this,         SLOT(accept()));
  layout()->addWidget(acceptButton);

  ESPINA_SETTINGS(settings);

  settings.beginGroup("Synaptic Apposition Surface Information Analysis");
  resize(settings.value("size", QSize (200, 200)).toSize());
  move  (settings.value("pos",  QPoint(200, 200)).toPoint());
  settings.endGroup();
}

//----------------------------------------------------------------------------
void SASAnalysisDialog::closeEvent(QCloseEvent *event)
{
  ESPINA_SETTINGS(settings);

  settings.beginGroup("Synaptic Apposition Surface Information Analysis");
  settings.setValue("size", size());
  settings.setValue("pos", pos());
  settings.endGroup();

  QDialog::closeEvent(event);
}

