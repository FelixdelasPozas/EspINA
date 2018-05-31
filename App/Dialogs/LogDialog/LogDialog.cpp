/*

 Copyright (C) 2018 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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
#include <App/Dialogs/LogDialog/LogDialog.h>
#include <GUI/Dialogs/DefaultDialogs.h>
#include <Support/Settings/Settings.h>

// Qt
#include <QClipboard>
#include <QTextCursor>

using namespace ESPINA;
using namespace ESPINA::GUI;

const QString SETTINGS_GROUP = "Log Dialog";

//--------------------------------------------------------------------
LogDialog::LogDialog()
: QDialog{DefaultDialogs::defaultParentWidget(), Qt::WindowFlags{Qt::WindowMaximizeButtonHint|Qt::WindowCloseButtonHint}}
{
  setupUi(this);

  setAttribute(Qt::WA_DeleteOnClose);

  connect(m_copy, SIGNAL(pressed()), this, SLOT(onCopyPressed()));

  ESPINA_SETTINGS(settings);
  settings.beginGroup(SETTINGS_GROUP);
  restoreGeometry(settings.value("geometry").toByteArray());
  settings.endGroup();
}

//--------------------------------------------------------------------
void LogDialog::closeEvent(QCloseEvent *ev)
{
  ESPINA_SETTINGS(settings);
  settings.beginGroup(SETTINGS_GROUP);
  settings.setValue("geometry", saveGeometry());
  settings.endGroup();
  settings.sync();

  QDialog::closeEvent(ev);
}

//--------------------------------------------------------------------
void LogDialog::addText(const QString& text)
{
  m_plainTextEdit->moveCursor(QTextCursor::End);
  m_plainTextEdit->insertPlainText(text);
  m_plainTextEdit->moveCursor(QTextCursor::End);
}

//--------------------------------------------------------------------
void LogDialog::setText(const QString& text)
{
  m_plainTextEdit->clear();
  addText(text);
}

//--------------------------------------------------------------------
void LogDialog::clear()
{
  m_plainTextEdit->clear();
}

//--------------------------------------------------------------------
void LogDialog::onCopyPressed()
{
  auto clipboard = QApplication::clipboard();
  clipboard->setText(m_plainTextEdit->toPlainText());
}
