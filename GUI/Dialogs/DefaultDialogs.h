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
#include <GUI/SupportedFormats.h>

// Qt
#include <QFileDialog>
#include <QMessageBox>
#include <QUrl>

class QUrl;
namespace ESPINA {
  namespace GUI {

    class EspinaGUI_EXPORT DefaultDialogs
    {
      /** \brief Returns the top level main window pointer for use of the dialogs.
       *
       */
      static QWidget *applicationCentralWidgetInstance();

    public:
      static QString DefaultPath();

    public:
      /** \brief Dialog for asking the user for a unspecified file.
       * \param[in] title of the dialog.
       * \param[in] filters dialog file selection filters.
       * \param[in] path file path.
       * \param[in] recent list of recently opened files to show in the dialog.
       * \param[in] parent parent widget.
       *
       * Returns the file name specified by the user.
       *
       */
      static QString OpenFile(const QString          &title,
                              const SupportedFormats &filters = SupportedFormats().addAllFormat(),
                              const QString          &path    = QString(),
                              const QList<QUrl>      &recent  = QList<QUrl>(),
                              QWidget                *parent  = applicationCentralWidgetInstance());

      /** \brief Dialog for asking the user for a unspecified group of files.
       * \param[in] title of the dialog.
       * \param[in] filters dialog file selection filters.
       * \param[in] path file path.
       * \param[in] recent list of recently opened files to show in the dialog.
       * \param[in] parent parent widget.
       *
       * Returns the file names specified by the user.
       *
       */
      static QStringList OpenFiles(const QString          &title,
                                   const SupportedFormats &filters = SupportedFormats().addAllFormat(),
                                   const QString          &path    = QString(),
                                   const QList<QUrl>      &recent  = QList<QUrl>(),
                                   QWidget                *parent  = applicationCentralWidgetInstance());

      /** \brief Dialog to select a directory to save files on
       * \param[in] title of the dialog.
       * \param[in] path file path.
       * \param[in] parent parent widget.
       *
       * Returns the path to the directory used to save files on
       *
       */
      static QDir SaveDirectory(const QString& title,
                                const QString& path   = QString(),
                                QWidget       *parent = applicationCentralWidgetInstance());

      /** \brief Dialog for saving a file.
       * \param[in] title of the dialog.
       * \param[in] filters file selection filters.
       * \param[in] path file path.
       * \param[in] suffix suggested file suffix.
       * \param[in] suggestion suggested file name.
       * \param[in] parent parent widget.
       *
       * Returns the file name specified by the user.
       *
       */
      static QString SaveFile(const QString&          title,
                              const SupportedFormats& filters    = SupportedFormats().addAllFormat(),
                              const QString&          path       = QString(),
                              const QString&          suffix     = QString(),
                              const QString&          suggestion = QString(),
                              QWidget                *parent     = applicationCentralWidgetInstance());

      /** \brief Dialog for saving a group of files.
       * \param[in] title title of the dialog.
       * \param[in] filters dialog file selection filters.
       * \param[in] path file path.
       * \param[in] suffix suggested file suffix.
       * \param[in] suggestion suggested file name.
       * \param[in] parent parent widget.
       *
       * Returns the file names specified by the user.
       *
       */
      static QStringList SaveFiles(const QString&          title,
                                   const SupportedFormats& filters    = SupportedFormats().addAllFormat(),
                                   const QString&          path       = QString(),
                                   const QString&          suffix     = QString(),
                                   const QString&          suggestion = QString(),
                                   QWidget                *parent     = applicationCentralWidgetInstance());

      static QString DefaultTitle();

      /** \brief Dialog to ask for user ok/cancel confirmation.
       * \param[in] title title of the dialog.
       * \param[in] message message to ask for confirmation.
       * \param[in] parent parent widget.
       *
       */
      static bool UserConfirmation(const QString& message,
                                   const QString& title   = DefaultDialogs::DefaultTitle(),
                                   QWidget       *parent  = applicationCentralWidgetInstance());

      /** \brief Dialog to inform the user.
       * \param[in] title title of the dialog.
       * \param[in] message message to show.
       *
       */
      static void InformationMessage(const QString& message,
                                     const QString& title  = DefaultDialogs::DefaultTitle(),
                                     QWidget       *parent = applicationCentralWidgetInstance());
    };
  } // namespace GUI
} // namespace ESPINA

#endif // ESPINA_GUI_DEFAULTDIALOGS_H
