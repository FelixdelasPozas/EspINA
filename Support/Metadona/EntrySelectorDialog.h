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

#ifndef ESPINA_ENTRY_SELECTOR_DIALOG_H
#define ESPINA_ENTRY_SELECTOR_DIALOG_H

#include <QDialog>
#include <ui_EntrySelectorDialog.h>
#include <Coordinator.h>

namespace EspINA
{
  class EntrySelectorDialog
  : public QDialog
  , private Ui::EntrySelectorDialog
  {
   Q_OBJECT

  public:
    explicit EntrySelectorDialog(const QString&             level,
                                 std::vector<Metadona::Id>& entries,
                                 QWidget*                   parent = 0,
                                 Qt::WindowFlags            f = 0);

    Metadona::Id selectedId() const
    { return m_selectedId; }

  private slots:
    void selectEntry();

    void createEntry();

  private:
    Metadona::Id m_selectedId;
  };

}

#endif // ESPINA_ENTRY_SELECTOR_DIALOG_H
