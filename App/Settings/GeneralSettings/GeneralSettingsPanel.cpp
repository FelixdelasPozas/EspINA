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
#include <App/Utils/UpdateCheck.h>

// Qt
#include <QToolButton>

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
  m_autosavePath      ->setText(QDir::toNativeSeparators(m_autoSave.path().absolutePath()));
  m_autosaveInterval  ->setValue(static_cast<int>(m_autoSave.interval()));
  m_autoSaveBackground->setChecked(m_autoSave.autoSaveInThread());
  m_loadSEGSettings   ->setChecked(m_settings->loadSEGfileSettings());
  m_temporalPath      ->setText(QDir::toNativeSeparators(m_settings->temporalPath()));
  m_doCheck           ->setChecked(m_settings->performAnalysisCheckOnLoad());
  m_updateCombo       ->setCurrentIndex(static_cast<int>(m_settings->updateCheckPeriodicity()));

  auto isSystemTemporalPath = (m_settings->temporalPath() == QDir::tempPath());
  m_systemPathCheckbox->setChecked(isSystemTemporalPath);
  m_temporalPath      ->setEnabled(!isSystemTemporalPath);
  m_temporalPathLabel ->setEnabled(!isSystemTemporalPath);
  m_browseDirButton   ->setEnabled(!isSystemTemporalPath);

  connect(m_browseDirButton, SIGNAL(clicked(bool)),
          this,              SLOT(onBrowseDirClicked()));

  connect(m_browseSaveDirButton, SIGNAL(clicked(bool)),
          this,                  SLOT(onBrowseDirClicked()));

  connect(m_systemPathCheckbox, SIGNAL(stateChanged(int)),
          this,                 SLOT(onTempDirCheckboxChangedState(int)));
}

//------------------------------------------------------------------------
void GeneralSettingsPanel::acceptChanges()
{
  m_settings->setUserName(m_userName->text());
  m_settings->setLoadSEGfileSettings(m_loadSEGSettings->isChecked());
  m_settings->setTemporalPath(m_temporalPath->text());
  m_settings->setPerformAnalysisCheckOnLoad(m_doCheck->isChecked());
  m_settings->setUpdateCheckPeriodicity(static_cast<Support::ApplicationSettings::UpdateCheckPeriodicity>(m_updateCombo->currentIndex()));
  m_autoSave.setPath(m_autosavePath->text());
  m_autoSave.setInterval(m_autosaveInterval->value());
  m_autoSave.setSaveInThread(m_autoSaveBackground->isChecked());
}

//------------------------------------------------------------------------
bool GeneralSettingsPanel::modified() const
{
  return (m_userName->text()                != m_settings->userName())
      || (m_autosavePath->text()            != QDir::toNativeSeparators(m_autoSave.path().absolutePath()))
      || (m_autosaveInterval->value()       != static_cast<int>(m_autoSave.interval()))
      || (m_autoSaveBackground->isChecked() != m_autoSave.autoSaveInThread())
      || (m_loadSEGSettings->isChecked()    != m_settings->loadSEGfileSettings())
      || (m_temporalPath->text()            != QDir::toNativeSeparators(m_settings->temporalPath()))
      || (m_doCheck->isChecked()            != m_settings->performAnalysisCheckOnLoad())
      || (m_updateCombo->currentIndex()     != static_cast<int>(m_settings->updateCheckPeriodicity()));
}

//------------------------------------------------------------------------
SettingsPanelPtr GeneralSettingsPanel::clone()
{
  return new GeneralSettingsPanel(m_autoSave,m_settings);
}

//------------------------------------------------------------------------
void GeneralSettingsPanel::onBrowseDirClicked()
{
  auto button = qobject_cast<QPushButton *>(sender());
  if(button)
  {
    QLineEdit *receiver = nullptr;
    if(button == m_browseDirButton) receiver = m_temporalPath;
    else                            receiver = m_autosavePath;

    QFileDialog dialog;
    dialog.setFileMode(QFileDialog::Directory);
    dialog.setOption(QFileDialog::ShowDirsOnly);
    if(receiver && !receiver->text().isEmpty()) dialog.setDirectory(receiver->text());

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
            receiver->setText(dir);
          }
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

//------------------------------------------------------------------------
void GeneralSettingsPanel::showEvent(QShowEvent* e)
{
  QWidget::showEvent(e);

  const auto labelWidth = m_intervalLabel->width();
  m_pathLabel->setMinimumWidth(labelWidth);
  m_temporalPathLabel->setMinimumWidth(labelWidth);
  m_nameLabel->setMinimumWidth(labelWidth);
  m_pathLabel->resize(labelWidth, m_pathLabel->height());
}
