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

#include <EspinaCore.h>

#include <QSettings>
#include <QDir>
#include <QTime>
#include <QPushButton>
#include <QMessageBox>
// #include <EspinaPluginManager.h>

//------------------------------------------------------------------------
GeneralSettingsPanel::GeneralSettingsPanel()
{
  setupUi(this);

  GeneralSettings &settings = EspinaCore::instance()->settings();

  samplePath->setText(settings.stackDirectory());
  userName->setText(settings.userName());
}

//------------------------------------------------------------------------
void GeneralSettingsPanel::acceptChanges()
{
  GeneralSettings &settings = EspinaCore::instance()->settings();

  settings.setStackDirectory(samplePath->text());
  settings.setUserName(userName->text());
}

//------------------------------------------------------------------------
bool GeneralSettingsPanel::modified() const
{
  GeneralSettings &settings = EspinaCore::instance()->settings();

  return samplePath->text() != settings.stackDirectory()
  || userName->text() != settings.userName();
}

//------------------------------------------------------------------------
ISettingsPanel *GeneralSettingsPanel::clone()
{
  return new GeneralSettingsPanel();
}

//------------------------------------------------------------------------
SettingsDialog::SettingsDialog(QWidget* parent, Qt::WindowFlags f)
: QDialog(parent, f)
, m_activePanel(NULL)
{
  setupUi(this);

  GeneralSettingsPanel *general = new GeneralSettingsPanel();
  addPanel(general);

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

