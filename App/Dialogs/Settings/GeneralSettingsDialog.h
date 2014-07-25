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


#ifndef ESPINA_GENERAL_SETTINGS_DIALOG_H
#define ESPINA_GENERAL_SETTINGS_DIALOG_H

#include <QDialog>

#include "ui_GeneralSettingsDialog.h"

#include <Core/EspinaTypes.h>
#include <Support/Settings/SettingsPanel.h>

namespace ESPINA
{
  class GeneralSettingsDialog
  : public QDialog
  , Ui::GeneralSettingsDialog
  {
    Q_OBJECT
  public:
    explicit GeneralSettingsDialog(QWidget *parent = 0,
                                   Qt::WindowFlags flags  = 0);

    virtual void accept();
    virtual void reject();

    void registerPanel(SettingsPanelSPtr panel);

  private:
    SettingsPanelSPtr panel(const QString &shortDesc);

  public slots:
    void changePreferencePanel(int panel);

  private:
    SettingsPanelPtr   m_activePanel;
    SettingsPanelSList m_panels;
  };

} // namespace ESPINA

#endif // ESPINA_GENERAL_SETTINGS_DIALOG_H
