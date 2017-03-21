/*
 *
 * Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 * This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

// ESPINA
#include "GeneralSettingsPanel.h"
#include <AutoSave.h>
#include <Core/Utils/EspinaException.h>
#include <GUI/Dialogs/DefaultDialogs.h>

using namespace ESPINA;
using namespace ESPINA::Core::Utils;
using namespace ESPINA::GUI;
using namespace ESPINA::Support::Settings;

//------------------------------------------------------------------------
GeneralSettingsPanel::GeneralSettingsPanel(AutoSave &autoSave, Support::GeneralSettingsSPtr settings)
: m_autoSave(autoSave)
, m_settings{settings}
{
  setupUi(this);

  m_userName          ->setText(m_settings->userName());
  m_autosavePath      ->setText(m_autoSave.path().absolutePath());
  m_autosaveInterval  ->setValue(m_autoSave.interval());
  m_autoSaveBackground->setChecked(autoSave.autoSaveInThread());
  m_loadSEGSettings   ->setChecked(m_settings->loadSEGfileSettings());
  m_temporalPath      ->setText(m_settings->temporalPath());
  m_doCheck           ->setChecked(m_settings->performAnalysisCheckOnLoad());

  auto isSystemTemporalPath = (m_settings->temporalPath() == QDir::tempPath());
  m_systemPathCheckbox->setChecked(isSystemTemporalPath);
  m_temporalPath      ->setEnabled(!isSystemTemporalPath);
  m_temporalPathLabel ->setEnabled(!isSystemTemporalPath);
  m_browseDirButton   ->setEnabled(!isSystemTemporalPath);

  connect(m_browseDirButton, SIGNAL(clicked(bool)),
          this,              SLOT(onBrowseDirClicked()));

  connect(m_systemPathCheckbox, SIGNAL(stateChanged(int)),
          this,                 SLOT(onTempDirCheckboxChangedState(int)));
}

//------------------------------------------------------------------------
GeneralSettingsPanel::~GeneralSettingsPanel()
{
}

//------------------------------------------------------------------------
void GeneralSettingsPanel::acceptChanges()
{
  m_settings->setUserName(m_userName->text());
  m_settings->setLoadSEGfileSettings(m_loadSEGSettings->isChecked());
  m_settings->setTemporalPath(m_temporalPath->text());
  m_settings->setPerformAnalysisCheckOnLoad(m_doCheck->isChecked());
  m_autoSave.setPath(m_autosavePath->text());
  m_autoSave.setInterval(m_autosaveInterval->value());
  m_autoSave.setSaveInThread(m_autoSaveBackground->isChecked());
}

//------------------------------------------------------------------------
void GeneralSettingsPanel::rejectChanges()
{
}

//------------------------------------------------------------------------
bool GeneralSettingsPanel::modified() const
{
  return m_userName->text()                != m_settings->userName()
      || m_loadSEGSettings->isChecked()    != m_settings->loadSEGfileSettings()
      || m_autosavePath->text()            != m_autoSave.path().absolutePath()
      || m_autosaveInterval->value()       != m_autoSave.interval()
      || m_temporalPath->text()            != m_settings->temporalPath()
      || m_doCheck->isChecked()            != m_settings->performAnalysisCheckOnLoad()
      || m_autoSaveBackground->isChecked() != m_autoSave.autoSaveInThread();
}

//------------------------------------------------------------------------
SettingsPanelPtr GeneralSettingsPanel::clone()
{
  return new GeneralSettingsPanel(m_autoSave,m_settings);
}

//------------------------------------------------------------------------
void GeneralSettingsPanel::onBrowseDirClicked()
{
  QFileDialog dialog;
  dialog.setFileMode(QFileDialog::Directory);
  dialog.setOption(QFileDialog::ShowDirsOnly);

  if(dialog.exec() == QFileDialog::Accepted)
  {
    auto dir = dialog.selectedFiles().first();

    QFileInfo info(dir);

    if(!info.isDir())
    {
      DefaultDialogs::InformationMessage(tr("%1 isn't a directory!").arg(dir), shortDescription(), "", this);
    }
    else
    {
      if(!info.isWritable() || !info.isReadable())
      {
        DefaultDialogs::InformationMessage(QString("Invalid permissions, can't write on directory %1!").arg(dir), shortDescription(), "", this);
      }
      else
      {
        if(!info.exists())
        {
          DefaultDialogs::InformationMessage(QString("The directory %1 doesn't exists!").arg(dir), shortDescription(), "", this);
        }
        else
        {
          m_temporalPath->setText(dir);
        }
      }
    }
  }
}

//------------------------------------------------------------------------
void GeneralSettingsPanel::onTempDirCheckboxChangedState(int state)
{
  auto enabled = (state == Qt::Checked);

  m_temporalPath->setEnabled(!enabled);
  m_temporalPathLabel->setEnabled(!enabled);
  m_browseDirButton->setEnabled(!enabled);

  if(enabled)
  {
    m_temporalPath->setText(QDir::tempPath());
  }
}
