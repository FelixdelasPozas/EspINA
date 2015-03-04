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

#include "GUI/EspinaGUI_Export.h"

// Qt
#include <QFileDialog>
#include <QMessageBox>

namespace ESPINA {
  namespace GUI {

    class SupportedFiles
    {
    public:
      SupportedFiles();

      SupportedFiles(const QString &name, const QString &extension);

      SupportedFiles &addFormat(const QString &name, const QString &extension);

      operator QString() const;

    private:
      void addFilter(const QString &name, const QString &extension);

    private:
      QString m_filter;
    };

    class EspinaGUI_EXPORT DefaultDialogs
    {
    public:
      /** \brief Dialog for asking the user for a unspecified file.
       * \param[in] title of the dialog.
       * \param[in] filters dialog file selection filters.
       * \param[in] path file path.
       *
       * Returns the file name specified by the user.
       *
       */
      static QString OpenFile(const QString& title,
                              const QString& filters = QString(),
                              const QString& path    = QString());

      /** \brief Dialog for asking the user for a unspecified group of files.
       * \param[in] title of the dialog.
       * \param[in] filters dialog file selection filters.
       * \param[in] path file path.
       *
       * Returns the file names specified by the user.
       *
       */
      static QStringList OpenFiles(const QString& title,
                                   const QString& filters = QString(),
                                   const QString& path    = QString());

      /** \brief Dialog for saving a file.
       * \param[in] title of the dialog.
       * \param[in] filters file selection filters.
       * \param[in] path file path.
       * \param[in] suffix suggested file suffix.
       * \param[in] suggestion suggested file name.
       *
       * Returns the file name specified by the user.
       *
       */
      static QString SaveFile(const QString& title,
                              const SupportedFiles& filters = SupportedFiles(QObject::tr("All"), "*"),
                              const QString& path           = QString(),
                              const QString& suffix         = QString(),
                              const QString& suggestion     = QString());

      /** \brief Dialog for saving a group of files.
       * \param[in] title title of the dialog.
       * \param[in] filters dialog file selection filters.
       * \param[in] path file path.
       * \param[in] suffix suggested file suffix.
       * \param[in] suggestion suggested file name.
       *
       * Returns the file names specified by the user.
       *
       */
      static QStringList SaveFiles(const QString& title,
                                   const SupportedFiles& filters = SupportedFiles(QObject::tr("All"), "*"),
                                   const QString& path           = QString(),
                                   const QString& suffix         = QString(),
                                   const QString& suggestion     = QString());

      /** \brief Dialog to ask for user ok/cancel confirmation.
       * \param[in] title title of the dialog.
       * \param[in] message message to ask for confirmation.
       *
       */
      static bool UserConfirmation(const QString& title, const QString& message);

      /** \brief Dialog to inform the user.
       * \param[in] title title of the dialog.
       * \param[in] message message to show.
       *
       */
      static void InformationMessage(const QString& title, const QString& message);
    };
  } // namespace GUI
} // namespace ESPINA

#endif // ESPINA_GUI_DEFAULTDIALOGS_H
