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

#ifndef APP_TOOLGROUPS_SESSION_LOGTOOL_H_
#define APP_TOOLGROUPS_SESSION_LOGTOOL_H_

// ESPINA
#include <Support/Widgets/ProgressTool.h>

// Qt
#include <QByteArray>

namespace ESPINA
{
  class EspinaMainWindow;
  class LogDialog;

  /** \class LogTool
   * \brief Implements an operations log that records and show the operations done on the current session.
   *
   */
  class LogTool
  : public Support::Widgets::ProgressTool
  {
      Q_OBJECT
    public:
      /** \brief LogTool class constructor.
       * \param[in] context Application context.
       * \param[in] window Main application window pointer.
       *
       */
      explicit LogTool(Support::Context &context, EspinaMainWindow *window);

      /** \brief LogTool class virtual destructor.
       *
       */
      virtual ~LogTool();

      virtual void saveSettings(std::shared_ptr<QSettings> settings) override;

    private slots:
      /** \brief Loads the old log from the loaded analysis.
       *
       */
      void onAnalysisLoaded();

      /** \brief Updates the log when an operation is done on the current analysis.
       *
       */
      void onOperationDone();

      /** \brief Opens/closes the log dialog.
       * \param[in] value True to show the log dialog and false to close it.
       *
       */
      void onToolToggled(bool value);

      /** \brief Nullifies the dialog pointer when the LogDialog is destroyed.
       *
       */
      void onDialogDestroyed();

    private:
      /** \brief Updates the tool status and tooltip.
       *
       */
      void updateToolStatus();

      /** \brief Appends a message to the log.
       * \param[in] message Text message.
       *
       */
      void appendMessage(const QString &message);

      QByteArray m_log;    /** log contents.    */
      int        m_index;  /** undostack index. */
      LogDialog *m_dialog; /** log dialog.      */
  };

} // namespace ESPINA

#endif // APP_TOOLGROUPS_SESSION_LOGTOOL_H_
