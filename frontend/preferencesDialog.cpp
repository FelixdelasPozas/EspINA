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
#include <EspinaPluginManager.h>

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

void ViewPreferences::addPanel(IPreferencePanel* panel)
{
  panels.push_back(panel);
}

//------------------------------------------------------------------------
QWidget* ViewPreferences::widget()
{
  if (panels.isEmpty())
    return new QWidget();
  
  QVBoxLayout *layout = new QVBoxLayout();
  foreach(IPreferencePanel *panel, panels)
  {
    QVBoxLayout *groupLayout = new QVBoxLayout();
    QGroupBox *group = new QGroupBox(panel->shortDescription());
    groupLayout->addWidget(panel->widget());
    group->setLayout(groupLayout);
    layout->addWidget(group);
  }
  QWidget *widget = new QWidget();
  widget->setLayout(layout);
  
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
  
  
  foreach(IPreferencePanel *panel, EspinaPluginManager::instance()->preferencePanels())
  {
    addPanel(panel);
  }
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
IPreferencePanel* PreferencesDialog::panel(const QString& shortDesc)
{
  foreach(IPreferencePanel *panel, m_panels)
  {
    if (panel->shortDescription() == shortDesc)
      return panel;
  }
  return NULL;
}


//------------------------------------------------------------------------
void PreferencesDialog::changePreferencePanel(int panel)
{
  longDescription->setText( m_panels[panel]->longDescription() );
  icon->setPixmap( m_panels[panel]->icon().pixmap(icon->size()) );
  scrollArea->setWidget(m_panels[panel]->widget());
}

