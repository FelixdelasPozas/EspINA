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

#ifndef ESPINA_SEGMENTATIONVISUALIZATIONSETTINGSDIALOG_H
#define ESPINA_SEGMENTATIONVISUALIZATIONSETTINGSDIALOG_H

#include "GUI/EspinaGUI_Export.h"

#include <QDialog>
#include <QStandardItemModel>
#include "GUI/ui_SegmentationVisualizationSettingsDialog.h"

class QStandardItem;

namespace EspINA
{
  class GraphicalRepresentationSettings;

  class EspinaGUI_EXPORT SegmentationVisualizationSettingsDialog
  : public QDialog
  , private Ui::SegmentationVisualizationSettingsDialog
  {
    Q_OBJECT
  public:
    typedef QMap<QStandardItem *, GraphicalRepresentationSettings *> Settings;
  public:
    explicit SegmentationVisualizationSettingsDialog(const QString  &title,
                                                     Settings       &settings,
                                                     QWidget        *parent = 0,
                                                     Qt::WindowFlags f = 0);
  private slots:
    void displayRepresentationSettings(QModelIndex index);

  private:
    Settings                        &m_settings;
    QStandardItemModel               m_representationsModel;
    GraphicalRepresentationSettings *m_currentSettings;
  };
}

#endif // ESPINA_SEGMENTATIONVISUALIZATIONSETTINGSDIALOG_H
