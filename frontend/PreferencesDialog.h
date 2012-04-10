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


#ifndef PREFERENCESDIALOG_H
#define PREFERENCESDIALOG_H

#include <QDialog>

#include <common/gui/IPreferencePanel.h>

#include "ui_GeneralPreferences.h"
#include "ui_PreferencesDialog.h"

class GeneralSettingsPanel : public ISettingsPanel, Ui::GeneralPreferences
{
public:
  GeneralSettingsPanel();

  virtual const QString shortDescription() {return "General";}
  virtual const QString longDescription() {return "General";}
  virtual const QIcon icon() {return QIcon(":/espina/editor.ico");}

  virtual void acceptChanges();
  virtual void rejectChanges();

  virtual QWidget* widget();
};

class PreferencesDialog : public QDialog, Ui::PreferencesDialog
{
  Q_OBJECT
public:
  explicit PreferencesDialog(QWidget* parent = 0, Qt::WindowFlags f = 0);

  virtual void accept();
  virtual void reject();

  void addPanel(ISettingsPanel* panel);
  ISettingsPanel * panel(const QString &shortDesc);


public slots:
  void changePreferencePanel(int panel);

private:
  ISettingsPanel         *m_activePanel;
  QList<ISettingsPanel *> m_panels;
};

#endif // PREFERENCESDIALOG_H
