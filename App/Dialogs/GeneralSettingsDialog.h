/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2011  Jorge Peña <jorge.pena.pastor@gmail.com>

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


#ifndef GENERALSETTINGSDIALOG_H
#define GENERALSETTINGSDIALOG_H

#include <QDialog>

#include <GUI/ISettingsPanel.h>

#include "ui_GeneralSettingsPanel.h"
#include "ui_GeneralSettingsDialog.h"

#include <Core/EspinaTypes.h>

namespace EspINA
{
  class ModelAdapter;
  class GeneralSettings;

  class GeneralSettingsPanel
  : public ISettingsPanel
  , Ui::GeneralSettingsPanel
  {
  public:
    GeneralSettingsPanel(ModelAdapter *model, GeneralSettings *settings);
    virtual ~GeneralSettingsPanel();

    virtual const QString shortDescription() {return "Session";}
    virtual const QString longDescription() {return "Session";}
    virtual const QIcon icon() {return QIcon(":/espina/editor.ico");}

    virtual void acceptChanges();
    virtual void rejectChanges();
    virtual bool modified() const;

    virtual ISettingsPanelPtr clone();

  private:
    ModelAdapter     *m_model;
    GeneralSettings *m_settings;
  };

  class GeneralSettingsDialog
  : public QDialog
  , Ui::GeneralSettingsDialog
  {
    Q_OBJECT
  public:
    explicit GeneralSettingsDialog(QWidget        *parent = 0,
                            Qt::WindowFlags flags  = 0);

    virtual void accept();
    virtual void reject();

    void registerPanel(ISettingsPanelPtr panel);

  private:
    ISettingsPanelPtr panel(const QString &shortDesc);

  public slots:
    void changePreferencePanel(int panel);

  private:
    ISettingsPanelPtr  m_activePanel;
    ISettingsPanelList m_panels;
  };

} // namespace EspINA

#endif // GENERALSETTINGSDIALOG_H
