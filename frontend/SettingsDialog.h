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

#include <common/settings/ISettingsPanel.h>

#include "ui_GeneralSettingsPanel.h"
#include "ui_SettingsDialog.h"

class GeneralSettingsPanel
: public ISettingsPanel
, Ui::GeneralSettingsPanel
{
public:
  GeneralSettingsPanel();

  virtual const QString shortDescription() {return "General";}
  virtual const QString longDescription() {return "General Settings";}
  virtual const QIcon icon() {return QIcon(":/espina/editor.ico");}

  virtual void acceptChanges();
  virtual bool modified() const;

  virtual ISettingsPanel *clone();
};

class SettingsDialog
: public QDialog
, Ui::SettingsDialog
{
  Q_OBJECT
public:
  explicit SettingsDialog(QWidget* parent = 0, Qt::WindowFlags f = 0);

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
