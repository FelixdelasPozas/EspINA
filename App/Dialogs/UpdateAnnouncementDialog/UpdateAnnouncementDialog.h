/*

 Copyright (C) 2019 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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

#ifndef APP_DIALOGS_UPDATEANNOUNCEMENTDIALOG_UPDATEANNOUNCEMENTDIALOG_H_
#define APP_DIALOGS_UPDATEANNOUNCEMENTDIALOG_UPDATEANNOUNCEMENTDIALOG_H_

// ESPINA
#include <ui_UpdateAnnouncementDialog.h>

// Qt
#include <QDialog>

namespace ESPINA
{
  /** \class UpdateAnnouncementDialog
   * \brief Dialog to show the version and description of an update.
   *
   */
  class UpdateAnnouncementDialog
  : public QDialog
  , private Ui::UpdateAnnouncementDialog
  {
      Q_OBJECT
    public:
      /** \brief UpdateAnnouncementDialog class constructor.
       *
       */
      explicit UpdateAnnouncementDialog();

      /** \brief UpdateAnnouncementDialog class virtual destructor.
       *
       */
      virtual ~UpdateAnnouncementDialog()
      {};

      /** \brief Sets the new version information.
       * \param[in] version Version string (major.minor.patch).
       * \param[in] releaseNotes Notes of past releses.
       *
       */
      void setVersionInformation(const QString &version, const QString &releaseNotes);

    private slots:
      /**\brief Closes the dialog and notes the new version as not to update or announce.
       *
       */
      void onSkipButtonPressed();

      /** \brief Closes the dialog and notes the new version to be reminded on next update check.
       *
       */
      void onRemindButtonPressed();

      /** \brief Closes the dialog and launches the web browser with the EspINA website.
       *
       */
      void onUpdateButtonPressed();

    private:
      /** \brief Helper method to connect widgets signals to slots.
       *
       */
      void connectSignals();

      QString m_version; /** new version string (major.minor.patch). */
  };

} // namespace ESPINA

#endif // APP_DIALOGS_UPDATEANNOUNCEMENTDIALOG_UPDATEANNOUNCEMENTDIALOG_H_
