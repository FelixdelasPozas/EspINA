/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2014  <copyright holder> <email>
 *
 * This program is free software: you can redistribute it and/or modify
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

#include "LevelSelectorDialog.h"

#include "ui_LevelSelectorDialog.h"

using namespace EspINA;


//------------------------------------------------------------------------
LevelSelectorDialog::LevelSelectorDialog(const std::vector<Metadona::Level>& levels,
                                         QWidget*                            parent,
                                         Qt::WindowFlags                     f)
: QDialog(parent, f)
{
  setupUi(this);

  for (auto& level : levels)
  {
    m_levelList->addItem(level.c_str());
  }

  m_levelList->setCurrentRow(0);

  connect(m_select, SIGNAL(clicked(bool)),
          this,     SLOT(selectLevel()));

  connect(m_stop, SIGNAL(clicked(bool)),
          this,   SLOT(stopSelection()));

  connect(m_cancel, SIGNAL(clicked(bool)),
          this,     SLOT(reject()));

  QApplication::setOverrideCursor(Qt::ArrowCursor);
}


//------------------------------------------------------------------------
LevelSelectorDialog::~LevelSelectorDialog()
{
  QApplication::restoreOverrideCursor();
}

//------------------------------------------------------------------------
void LevelSelectorDialog::selectLevel()
{
  m_selectedLevel = m_levelList->selectedItems().first()->text().toStdString();

  accept();
}

//------------------------------------------------------------------------
void LevelSelectorDialog::stopSelection()
{
  m_selectedLevel = "";

  accept();
}