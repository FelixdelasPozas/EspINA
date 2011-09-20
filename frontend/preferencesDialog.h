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
#include "IPreferencePanel.h"

#include "ui_GeneralPreferences.h"
#include "ui_ViewPreferences.h"
#include "ui_preferencesDialog.h"
/*
class IPreferencePanel : public QWidget
{
public:
  virtual ~IPreferencePanel(){}
  
  virtual const QString shortDescription() = 0;
  virtual const QString longDescription() = 0;
  virtual const QIcon icon() = 0;  
  
  virtual QWidget *widget() = 0;
};*/

class GeneralPreferences : public IPreferencePanel, Ui::GeneralPreferences
{
public:
  GeneralPreferences();
  
  virtual const QString shortDescription() {return "General";}
  virtual const QString longDescription() {return "General";}
  virtual const QIcon icon() {return QIcon(":/espina/editor.ico");}
  
  virtual QWidget* widget();
};

class ViewPreferences : public IPreferencePanel, Ui::ViewPreferences
{
public:
  ViewPreferences();
  
  virtual const QString shortDescription() {return "View";}
  virtual const QString longDescription() {return "View";}
  virtual const QIcon icon() {return QIcon(":/espina/show_all.svg");}
  
  virtual QWidget* widget();
};

class PreferencesDialog : public QDialog, Ui::PreferencesDialog
{
  Q_OBJECT
  
public:
  explicit PreferencesDialog(QWidget* parent = 0, Qt::WindowFlags f = 0);
  
  void addPanel(IPreferencePanel* panel);
  
public slots:
  void changePreferencePanel(int panel);
  
private:
  QList<IPreferencePanel *> m_panels;
};

#endif // PREFERENCESDIALOG_H
