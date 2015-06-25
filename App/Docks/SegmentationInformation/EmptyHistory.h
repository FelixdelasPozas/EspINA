/*
 * Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>
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

#ifndef ESPINA_EMPTY_HISTORY_H
#define ESPINA_EMPTY_HISTORY_H

// Qt
#include <QWidget>

#include "ui_EmptyHistory.h"

namespace ESPINA
{
  class EmptyHistory
  : public QWidget
  {
    Q_OBJECT
  public:
    explicit EmptyHistory(QWidget* parent = 0, Qt::WindowFlags f = 0);

  private:
    Ui::EmptyHistory *m_gui;
  };

} // namespace ESPINA

#endif // ESPINA_EMPTY_HISTORY_H
