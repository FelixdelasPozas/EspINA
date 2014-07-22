/*
    Copyright (c) 2013, Jorge Peña Pastor <jpena@cesvima.upm.es>
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
        * Redistributions of source code must retain the above copyright
        notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.
        * Neither the name of the <organization> nor the
        names of its contributors may be used to endorse or promote products
        derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY Jorge Peña Pastor <jpena@cesvima.upm.es> ''AS IS'' AND ANY
    EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL Jorge Peña Pastor <jpena@cesvima.upm.es> BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#include "NoteEditor.h"

#include <ui_NoteEditor.h>

#include <QFileDialog>
#include <QTextStream>

using namespace ESPINA;

class NoteEditor::GUI
: public  Ui::NoteEditor
{
};

//----------------------------------------------------------------------------
NoteEditor::NoteEditor(const QString &title,
                       const QString &text,
                       QWidget *parent,
                       Qt::WindowFlags f)
: QDialog(parent, f)
, m_gui(new GUI())
{
  m_gui->setupUi(this);

  setWindowTitle(tr("%1 Notes").arg(title));
  setWindowIcon(QIcon(":/espina/note.png"));

  m_gui->textEdit->setText(text);

  QIcon saveIcon = qApp->style()->standardIcon(QStyle::SP_DialogSaveButton);
  m_gui->saveButton->setIcon(saveIcon);
  connect(m_gui->saveButton, SIGNAL(clicked(bool)),
          this, SLOT(exportNote()));

  connect(m_gui->acceptChanges, SIGNAL(clicked(bool)),
          this, SLOT(accept()));
  connect(m_gui->rejectChanges, SIGNAL(clicked(bool)),
          this, SLOT(reject()));
}

//----------------------------------------------------------------------------
NoteEditor::~NoteEditor()
{
  delete m_gui;
}

//----------------------------------------------------------------------------
QString NoteEditor::text()
{
  return m_gui->textEdit->document()->toPlainText();
}

//----------------------------------------------------------------------------
void NoteEditor::exportNote()
{
  QString fileName = QFileDialog::getSaveFileName(this,
                                                  tr("Export %1").arg(windowTitle()),
                                                  QString("%1.txt").arg(windowTitle()),
                                                  tr("Text File (*.txt)"));
  if (fileName.isEmpty())
    return;

  QFile file(fileName);
  file.open(QIODevice::WriteOnly |  QIODevice::Text);
  QTextStream out(&file);

  out << m_gui->textEdit->toPlainText();
  file.close();
}
