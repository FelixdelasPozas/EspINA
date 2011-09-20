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


#include "preferencesDialog.h"

#include <QSettings>
#include <QDir>
#include <QTime>
#include <QPushButton>

const QString SAMPLE_PATH("samplePath");

//------------------------------------------------------------------------
GeneralPreferences::GeneralPreferences()
{
}

//------------------------------------------------------------------------
QWidget* GeneralPreferences::widget()
{
  QWidget *widget = new QWidget();
  setupUi(widget);

  QSettings settings;
  
  if (!settings.allKeys().contains(SAMPLE_PATH))
  {
    settings.setValue(SAMPLE_PATH,QDir::homePath()+"/Stacks");
  }
  
  samplePath->setText(settings.value(SAMPLE_PATH).toString());
  
  return widget;
}

//------------------------------------------------------------------------
ViewPreferences::ViewPreferences()
{

}

//------------------------------------------------------------------------
QWidget* ViewPreferences::widget()
{
  QWidget *widget = new QWidget();
  setupUi(widget);
  return widget;
}

//------------------------------------------------------------------------
PreferencesDialog::PreferencesDialog(QWidget* parent, Qt::WindowFlags f): QDialog(parent, f)
{
  setupUi(this);
  
  GeneralPreferences *general = new GeneralPreferences();
  addPanel(general);

  ViewPreferences *view = new ViewPreferences();
  addPanel(view);
  
  connect(components,SIGNAL(currentRowChanged(int)),
	  this, SLOT(changePreferencePanel(int)));
}

//------------------------------------------------------------------------
void PreferencesDialog::addPanel(IPreferencePanel* panel)
{
  QListWidgetItem *item = new QListWidgetItem();
  item->setData(Qt::DisplayRole,panel->shortDescription());
  item->setData(Qt::DecorationRole,panel->icon());

  components->addItem(item);
  m_panels.push_back(panel);
}

//------------------------------------------------------------------------
void PreferencesDialog::changePreferencePanel(int panel)
{
  longDescription->setText( m_panels[panel]->longDescription() );
  icon->setPixmap( m_panels[panel]->icon().pixmap(icon->size()) );
  scrollArea->setWidget(m_panels[panel]->widget());
}

