/*

 Copyright (C) 2019 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

 This file is part of ESPINA.

 ESPINA is free software: you can redistribute it and/or modify
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

// ESPINA
#include <EspinaConfig.h>
#include <App/Dialogs/UpdateAnnouncementDialog/UpdateAnnouncementDialog.h>
#include <App/Utils/UpdateCheck.h>
#include <GUI/Dialogs/DefaultDialogs.h>
#include <Support/Settings/Settings.h>

// Qt
#include <QDesktopServices>
#include <QTextCursor>
#include <QTextListFormat>

using namespace ESPINA;
using namespace GUI;

const QUrl ESPINA_URL{"http://cajalbbp.cesvima.upm.es/espina/#started"};

//--------------------------------------------------------------------
UpdateAnnouncementDialog::UpdateAnnouncementDialog()
: QDialog{DefaultDialogs::defaultParentWidget(), Qt::WindowFlags()}
{
  setupUi(this);

  connectSignals();
}

//--------------------------------------------------------------------
void UpdateAnnouncementDialog::setVersionInformation(const QString& version, const QString& releaseNotes)
{
  m_version = version;
  m_versionComparison->setText(m_versionComparison->text().arg(version).arg(ESPINA_VERSION));
  m_releaseNotesText->setAcceptRichText(true);

  auto text = releaseNotes;
  text.replace("\n", "<br/>");
  m_releaseNotesText->setHtml(text);

  // Go to begin of the text.
  auto textCursor = m_releaseNotesText->textCursor();
  textCursor.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor,1);
  m_releaseNotesText->setTextCursor(textCursor);
  m_releaseNotesText->ensureCursorVisible();
}

//--------------------------------------------------------------------
void UpdateAnnouncementDialog::onSkipButtonPressed()
{
  close();

  ESPINA_SETTINGS(settings);

  settings.setValue(UpdateCheck::SKIP_VERSION_KEY, m_version);
  settings.sync();
}

//--------------------------------------------------------------------
void UpdateAnnouncementDialog::onRemindButtonPressed()
{
  close();
}

//--------------------------------------------------------------------
void UpdateAnnouncementDialog::onUpdateButtonPressed()
{
  close();

  QDesktopServices::openUrl(ESPINA_URL);
}

//--------------------------------------------------------------------
void UpdateAnnouncementDialog::connectSignals()
{
  connect(m_laterButton,  SIGNAL(pressed()), this, SLOT(onRemindButtonPressed()));
  connect(m_skipButton,   SIGNAL(pressed()), this, SLOT(onSkipButtonPressed()));
  connect(m_updateButton, SIGNAL(pressed()), this, SLOT(onUpdateButtonPressed()));
}
