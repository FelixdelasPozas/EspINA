/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2013  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 * This program is free software: you can redistribute it and/or modify
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

#include "SegmentationVisualizationSettingsDialog.h"
#include <GUI/Representations/GraphicalRepresentationSettings.h>
#include <QDebug>

using namespace EspINA;

//----------------------------------------------------------------------------
SegmentationVisualizationSettingsDialog::SegmentationVisualizationSettingsDialog(const QString  &title, 
                                                                                 Settings       &settings,
                                                                                 QWidget        *parent,
                                                                                 Qt::WindowFlags f)
: QDialog(parent, f)
, m_settings(settings)
, m_currentSettings(NULL)
{
  setupUi(this);

  connect(m_acceptChanges, SIGNAL(clicked(bool)),
          this, SLOT(accept()));
  connect(m_rejectChanges, SIGNAL(clicked(bool)),
          this, SLOT(reject()));

  foreach(QStandardItem *representation, settings.keys())
  {
    representation->setEditable(false);

    m_representationsModel.appendRow(representation);
    m_representationsModel.sort(0);

    m_representationList->setModel(&m_representationsModel);
    connect(m_representationList, SIGNAL(clicked(QModelIndex)),
            this, SLOT(displayRepresentationSettings(QModelIndex)));
  }

  m_representationList->selectionModel()->select(m_representationsModel.index(0,0), QItemSelectionModel::Select|QItemSelectionModel::Current);
  displayRepresentationSettings(m_representationsModel.index(0, 0));
}

//----------------------------------------------------------------------------
void SegmentationVisualizationSettingsDialog::displayRepresentationSettings(QModelIndex index)
{
  if (m_currentSettings)
  {
    m_settingsLayout->removeWidget(m_currentSettings);
    m_currentSettings->setParent(NULL);
  }

  int row = index.row();
  m_currentSettings = m_settings[m_representationsModel.item(row)];

  m_settingsLayout->addWidget(m_currentSettings);
}
