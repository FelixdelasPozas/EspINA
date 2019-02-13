/*

 Copyright (C) 2018 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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

#ifndef APP_DIALOGS_LOGDIALOG_LOGDIALOG_H_
#define APP_DIALOGS_LOGDIALOG_LOGDIALOG_H_

// ESPINA
#include "ui_LogDialog.h"

// Qt
#include <QDialog>

namespace ESPINA
{
  /** \class LogDialog
   * \brief Implements a dialog to show the operations log.
   *
   */
  class LogDialog
  : public QDialog
  , private Ui::LogDialog
  {
      Q_OBJECT
    public:
      /** \brief LogDialog class constructor.
       *
       */
      explicit LogDialog();

      /** \brief LogDialog class virtual destructor.
       *
       */
      virtual ~LogDialog()
      {};

      /** \brief Adds the given text to the window text.
       * \param[in] text QString object.
       *
       */
      void addText(const QString &text);

      /** \brief Replaces the window text with the given one.
       * \param[in] text QString object.
       *
       */
      void setText(const QString &text);

      /** \brief Clears the text of the dialog.
       *
       */
      void clear();

    protected:
      virtual void closeEvent(QCloseEvent *e) override;

    private slots:
      /** \brief Copies the window text to the system clipboard.
       *
       */
      void onCopyPressed();

      /** \brief Performs a search on the log text.
       *
       */
      void onSearchButtonPressed();

      /** \brief Enables/disables the search button depending on its contents.
       *
       */
      void onSearchLineModified();

    private:
      /** \brief Removes the text highlights and clears the dirty flag.
       *
       */
      void clearDirtyFlag();

      bool m_dirty; /** true if there is a search highlighted on the log, false otherwise. */
  };

} // namespace ESPINA

#endif // APP_DIALOGS_LOGDIALOG_LOGDIALOG_H_
