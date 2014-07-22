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

#ifndef ESPINA_GUI_DEFAULTDIALOGS_H
#define ESPINA_GUI_DEFAULTDIALOGS_H

#include <QFileDialog>
#include <QMessageBox>

namespace ESPINA {
  namespace GUI {

    class DefaultDialogs
    {
    public:
      static QString OpenFile(const QString& title,
                              const QString& filters = QString(),
                              const QString& path    = QString());

      static QStringList OpenFiles(const QString& title,
                                   const QString& filters = QString(),
                                   const QString& path    = QString());

      static QString SaveFile(const QString& title,
                              const QString& filters    = QString(),
                              const QString& path       = QString(),
                              const QString& suffix     = QString(),
                              const QString& suggestion = QString());

      static QStringList SaveFiles(const QString& title,
                                   const QString& filters    = QString(),
                                   const QString& path       = QString(),
                                   const QString& suffix     = QString(),
                                   const QString& suggestion = QString());

      static bool UserConfirmation(const QString& title, const QString& message);
    };
  } // namespace GUI
} // namespace ESPINA

#endif // ESPINA_GUI_DEFAULTDIALOGS_H
