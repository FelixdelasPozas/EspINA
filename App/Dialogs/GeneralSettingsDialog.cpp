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


#include "GeneralSettingsDialog.h"

#include "Settings/GeneralSettings.h"
#include <Core/Model/EspinaModel.h>
#include <Core/Model/Filter.h>

#include <QDir>
#include <QMessageBox>
#include <QPushButton>
#include <QSettings>
#include <QTime>

#include <QDebug>
using namespace EspINA;

//------------------------------------------------------------------------
GeneralSettingsPanel::GeneralSettingsPanel(ModelAdapter *model, GeneralSettings *settings)
: m_model(model)
, m_settings(settings)
{
  setupUi(this);

  bool useAnalysisTraceability = !m_model->filters().isEmpty();

  if (useAnalysisTraceability)
  {
    traceabilty->setChecked(m_model->isTraceable());

    int i = 0;
    bool isTraceable = true;
    while (isTraceable && i < m_model->filters().size())
    {
      isTraceable &= m_model->filters()[i]->isTraceable();
      ++i;
    }
    traceabilty->setEnabled(isTraceable);
  }
  else
    traceabilty->setChecked(m_settings->useTraceability());


  userName->setText(m_settings->userName());
  autosavePath->setText(m_settings->autosavePath().absolutePath());
  autosaveInterval->setValue(m_settings->autosaveInterval());
}

//------------------------------------------------------------------------
GeneralSettingsPanel::~GeneralSettingsPanel()
{
  //qDebug() << "Destroying General Settings Panel";
}

//------------------------------------------------------------------------
void GeneralSettingsPanel::acceptChanges()
{
  if (m_model->filters().isEmpty())
    m_settings->setUseTraceability(traceabilty->isChecked());
  else
    m_model->setTraceable(traceabilty->isChecked());

  m_settings->setUserName(userName->text());
  m_settings->setAutosavePath(autosavePath->text());
  m_settings->setAutosaveInterval(autosaveInterval->value());
}

//------------------------------------------------------------------------
void GeneralSettingsPanel::rejectChanges()
{

}

//------------------------------------------------------------------------
bool GeneralSettingsPanel::modified() const
{
  bool traceabiltyValue = m_model->filters().isEmpty()
                        ? m_settings->useTraceability()
                        : m_model->isTraceable();

  return traceabilty->isChecked()  != traceabiltyValue
      || userName->text()          != m_settings->userName()
      || autosavePath->text()      != m_settings->autosavePath().absolutePath()
      || autosaveInterval->value() != m_settings->autosaveInterval();
}

//------------------------------------------------------------------------
ISettingsPanelPtr GeneralSettingsPanel::clone()
{
  return ISettingsPanelPtr(new GeneralSettingsPanel(m_model, m_settings));
}




//------------------------------------------------------------------------
GeneralSettingsDialog::GeneralSettingsDialog(QWidget *parent, Qt::WindowFlags flags)
: QDialog(parent, flags)
, m_activePanel(NULL)
{
  setupUi(this);

  setWindowTitle(tr("EspINA Settings"));

  connect(components,SIGNAL(currentRowChanged(int)),
          this, SLOT(changePreferencePanel(int)));
}

//------------------------------------------------------------------------
void GeneralSettingsDialog::accept()
{
  m_activePanel->acceptChanges();
  QDialog::accept();
}

//------------------------------------------------------------------------
void GeneralSettingsDialog::reject()
{
  m_activePanel->rejectChanges();
  QDialog::reject();
}

//------------------------------------------------------------------------
void GeneralSettingsDialog::registerPanel(ISettingsPanelPtr panel)
{
  QListWidgetItem *item = new QListWidgetItem();
  item->setData(Qt::DisplayRole,panel->shortDescription());
  item->setData(Qt::DecorationRole,panel->icon());

  components->addItem(item);
  m_panels << panel;
}

//------------------------------------------------------------------------
ISettingsPanelPtr GeneralSettingsDialog::panel(const QString& shortDesc)
{
  foreach(ISettingsPanelPtr panel, m_panels)
  {
    if (panel->shortDescription() == shortDesc)
      return panel;
  }

  Q_ASSERT(false);
  return NULL;
}


//------------------------------------------------------------------------
void GeneralSettingsDialog::changePreferencePanel(int panel)
{
  if (m_activePanel && m_activePanel->modified())
  {
    QMessageBox msg;
    msg.setWindowTitle(tr("EspINA"));
    msg.setText(tr("Settings panel \"%1\" have been modified.\nDo you want to save the changes?").arg(m_activePanel->shortDescription()));
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
