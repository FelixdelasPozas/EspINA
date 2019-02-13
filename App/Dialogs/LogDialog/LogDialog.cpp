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
#include <QDebug>

using namespace ESPINA;
using namespace ESPINA::GUI;

const QString SETTINGS_GROUP = "Log Dialog";

//--------------------------------------------------------------------
LogDialog::LogDialog()
: QDialog{DefaultDialogs::defaultParentWidget(), Qt::WindowFlags{Qt::WindowMaximizeButtonHint|Qt::WindowCloseButtonHint}}
, m_dirty{false}
{
  setupUi(this);

  setAttribute(Qt::WA_DeleteOnClose);

  connect(m_copy,         SIGNAL(pressed()),                    this, SLOT(onCopyPressed()));
  connect(m_searchButton, SIGNAL(pressed()),                    this, SLOT(onSearchButtonPressed()));
  connect(m_searchLine,   SIGNAL(textChanged(const QString &)), this, SLOT(onSearchLineModified()));

  ESPINA_SETTINGS(settings);
  settings.beginGroup(SETTINGS_GROUP);
  restoreGeometry(settings.value("geometry").toByteArray());
  settings.endGroup();
}

//--------------------------------------------------------------------
void LogDialog::closeEvent(QCloseEvent *e)
{
  ESPINA_SETTINGS(settings);
  settings.beginGroup(SETTINGS_GROUP);
  settings.setValue("geometry", saveGeometry());
  settings.endGroup();
  settings.sync();

  QDialog::closeEvent(e);
}

//--------------------------------------------------------------------
void LogDialog::addText(const QString& text)
{
  clearDirtyFlag();

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
  m_dirty = false;
  m_searchText->setText(tr("No search has been performed."));
}

//--------------------------------------------------------------------
void LogDialog::onCopyPressed()
{
  auto clipboard = QApplication::clipboard();
  clipboard->setText(m_plainTextEdit->toPlainText());
}

//--------------------------------------------------------------------
void LogDialog::onSearchButtonPressed()
{
  static QString text;
  static unsigned long count = 0;
  static unsigned long max   = 0;

  m_plainTextEdit->updateGeometry();

  if(text == m_searchLine->text())
  {
    ++count;
    if(count >= max) count = 0;
  }
  else
  {
    text  = m_searchLine->text();
    count = 0;
  }

  auto document       = m_plainTextEdit->document();
  auto previousCursor = m_plainTextEdit->textCursor();
  unsigned int found  = 0;

  clearDirtyFlag();

  QTextCursor highlightCursor(document);
  QTextCursor cursor(document);
  cursor.beginEditBlock();

  QTextCharFormat highlight(highlightCursor.charFormat());
  highlight.setForeground(Qt::red);

  QTextCharFormat highbackground(highlightCursor.charFormat());
  highbackground.setForeground(Qt::white);
  highbackground.setBackground(Qt::red);

  while (!highlightCursor.isNull() && !highlightCursor.atEnd())
  {
    highlightCursor = document->find(text, highlightCursor, QTextDocument::FindCaseSensitively);

    if (!highlightCursor.isNull())
    {
      if(found != count)
      {
        highlightCursor.setCharFormat(highlight);
      }
      else
      {
        highlightCursor.setCharFormat(highbackground);
        highlightCursor.clearSelection();

        m_plainTextEdit->setTextCursor(highlightCursor);
      }

      ++found;
    }
  }

  cursor.endEditBlock();

  QString message;

  if(found == 0)
  {
    message = tr("There are no occurrences of '%1'.").arg(text);
  }
  else
  {
    message = tr("Found %1 occurrences of '%2' (selected occurrence %3).").arg(found).arg(text).arg(count+1);
  }

  m_searchText->setText(message);

  max = found;

  m_dirty = (found != 0);
}

//--------------------------------------------------------------------
void LogDialog::onSearchLineModified()
{
  auto text = m_searchLine->text();

  m_searchButton->setEnabled(!text.isEmpty());

  if(text.isEmpty())
  {
    m_searchText->setText(tr("No search has been performed."));
  }
  else
  {
    m_searchText->setText(tr("Click on search button to find occurrences of '%1' in the log.").arg(text));
  }

  clearDirtyFlag();
}

//--------------------------------------------------------------------
void LogDialog::clearDirtyFlag()
{
  if(m_dirty)
  {
    m_plainTextEdit->document()->undo();
    m_dirty = false;
  }
}
