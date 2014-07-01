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

#include "EntrySelectorDialog.h"

#include "ui_EntrySelectorDialog.h"

using namespace EspINA;


//------------------------------------------------------------------------
EntrySelectorDialog::EntrySelectorDialog(const QString&             level,
                                         std::vector<Metadona::Id>& entries,
                                         QWidget*                   parent,
                                         Qt::WindowFlags            f)
: QDialog(parent, f)
{
  setupUi(this);

  m_label->setText(level + ":");

  for (auto& entry : entries)
  {
    m_entryList->addItem(entry.c_str());
  }

  m_entryList->setCurrentRow(0);

  connect(m_create, SIGNAL(clicked(bool)),
          this,     SLOT(createEntry()));

  connect(m_select, SIGNAL(clicked(bool)),
          this,     SLOT(selectEntry()));

  connect(m_cancel, SIGNAL(clicked(bool)),
          this,     SLOT(reject()));

  QApplication::setOverrideCursor(Qt::ArrowCursor);
}

//------------------------------------------------------------------------
EntrySelectorDialog::~EntrySelectorDialog()
{
  QApplication::restoreOverrideCursor();
}

//------------------------------------------------------------------------
void EntrySelectorDialog::createEntry()
{
  m_selectedId = "";

  accept();
}

//------------------------------------------------------------------------
void EntrySelectorDialog::selectEntry()
{
  m_selectedId = m_entryList->selectedItems().first()->text().toStdString();
  accept();
}

