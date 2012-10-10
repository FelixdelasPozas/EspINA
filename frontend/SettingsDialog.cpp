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


#include "SettingsDialog.h"

#include "common/settings/GeneralSettings.h"

#include <QDir>
#include <QMessageBox>
#include <QPushButton>
#include <QSettings>
#include <QTime>

//------------------------------------------------------------------------
GeneralSettingsPanel::GeneralSettingsPanel(GeneralSettings *settings)
: m_settings(settings)
{
  setupUi(this);

  userName->setText(m_settings->userName());
  autosavePath->setText(m_settings->autosavePath().absolutePath());
  autosaveInterval->setValue(m_settings->autosaveInterval());
}

//------------------------------------------------------------------------
void GeneralSettingsPanel::acceptChanges()
{
  m_settings->setUserName(userName->text());
  m_settings->setAutosavePath(autosavePath->text());
  m_settings->setAutosaveInterval(autosaveInterval->value());
}

//------------------------------------------------------------------------
bool GeneralSettingsPanel::modified() const
{
  return userName->text() != m_settings->userName()
      || autosavePath->text() != m_settings->autosavePath().absolutePath()
      || autosaveInterval->value() != m_settings->autosaveInterval();
}

//------------------------------------------------------------------------
ISettingsPanel *GeneralSettingsPanel::clone()
{
  return new GeneralSettingsPanel(m_settings);
}

//------------------------------------------------------------------------
SettingsDialog::SettingsDialog(QWidget* parent, Qt::WindowFlags f)
: QDialog(parent, f)
, m_activePanel(NULL)
{
  setupUi(this);

  connect(components,SIGNAL(currentRowChanged(int)),
	  this, SLOT(changePreferencePanel(int)));

//   foreach(IPreferencePanel *panel, EspinaPluginManager::instance()->preferencePanels())
//   {
//     addPanel(panel);
//   }
}

//------------------------------------------------------------------------
void SettingsDialog::accept()
{
  m_activePanel->acceptChanges();
  QDialog::accept();
}

//------------------------------------------------------------------------
void SettingsDialog::reject()
{
  m_activePanel->rejectChanges();
  QDialog::reject();
}

//------------------------------------------------------------------------
void SettingsDialog::addPanel(ISettingsPanel* panel)
{
  QListWidgetItem *item = new QListWidgetItem();
  item->setData(Qt::DisplayRole,panel->shortDescription());
  item->setData(Qt::DecorationRole,panel->icon());

  components->addItem(item);
  m_panels.push_back(panel);
}

//------------------------------------------------------------------------
ISettingsPanel* SettingsDialog::panel(const QString& shortDesc)
{
  foreach(ISettingsPanel *panel, m_panels)
  {
    if (panel->shortDescription() == shortDesc)
      return panel;
  }
  return NULL;
}


//------------------------------------------------------------------------
void SettingsDialog::changePreferencePanel(int panel)
{
  if (m_activePanel && m_activePanel->modified())
  {
    QMessageBox msg;
    msg.setText(tr("Settings have changed. Do you want to save them"));
    msg.setStandardButtons(QMessageBox::Yes|QMessageBox::No);
    if (msg.exec() == QMessageBox::Yes)
      m_activePanel->acceptChanges();
    else
      m_activePanel->rejectChanges();
  }

  m_activePanel = m_panels[panel]->clone();
  longDescription->setText( m_activePanel->longDescription() );
  icon->setPixmap( m_activePanel->icon().pixmap(icon->size()) );
  scrollArea->setWidget(m_activePanel);
}