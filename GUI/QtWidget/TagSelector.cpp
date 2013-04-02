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


#include "TagSelector.h"

#include <ui_TagSelector.h>

#include <QFileDialog>
#include <qinputdialog.h>
#include <QTextStream>

using namespace EspINA;

class TagSelector::GUI
: public  Ui::TagSelector
{
};

//----------------------------------------------------------------------------
TagSelector::TagSelector(const QString &title,
                         QStandardItemModel &tags,
                         QWidget *parent,
                         Qt::WindowFlags f)
: QDialog(parent, f)
, m_gui(new GUI())
, m_tags(tags)
{
  m_gui->setupUi(this);

  setWindowTitle(tr("%1 Tags").arg(title));
  setWindowIcon(QIcon(":/espina/tag.svg"));

  m_tags.sort(0);
  m_gui->tagList->setModel(&tags);

  connect(m_gui->createTagButton, SIGNAL(clicked(bool)),
          this, SLOT(createTag()));

  connect(m_gui->acceptChanges, SIGNAL(clicked(bool)),
          this, SLOT(accept()));
  connect(m_gui->rejectChanges, SIGNAL(clicked(bool)),
          this, SLOT(reject()));
}

//----------------------------------------------------------------------------
TagSelector::~TagSelector()
{
  delete m_gui;
}

//----------------------------------------------------------------------------
void TagSelector::createTag()
{
  bool ok;
  QString newTag = QInputDialog::getText(this,
                                         tr("Add New Tag"),
                                            tr("Introduce tag name"),
                                         QLineEdit::Normal,
                                         "",
                                         &ok);
  if (ok)
  {
    newTag = newTag.toLower();

    bool found = false;
    for (int r = 0; r < m_tags.rowCount(); ++r)
      if (m_tags.item(r)->text() == newTag)
        found = true;

    if (!found)
    {
      QStandardItem *item = new QStandardItem(newTag);
      item->setCheckable(true);
      item->setCheckState(Qt::Checked);

      m_tags.appendRow(item);

      m_tags.sort(0);
    }
  }
}
