/*

 Copyright (C) 2015 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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

#ifndef APP_TOOLGROUPS_FILE_FILESAVETOOL_H_
#define APP_TOOLGROUPS_FILE_FILESAVETOOL_H_

// ESPINA
#include <Core/Analysis/Analysis.h>
#include <Settings/GeneralSettings/GeneralSettings.h>
#include <Support/Widgets/ProgressTool.h>
#include "EspinaErrorHandler.h"

// Qt
#include <QTimer>

namespace ESPINA
{
  //----------------------------------------------------------------------------
  class FileSaveTool
  : public Support::Widgets::ProgressTool
  {
      Q_OBJECT
    public:
      /** \brief FileSaveTool class constructor.
       * \param[in] context application context.
       * \param[in] analysis application analysis pointer.
       * \param[in] errorHandler application error handler.
       * \param[in] settings application general settings.
       *
       */
      explicit FileSaveTool(Support::Context &context, AnalysisSPtr analysis, EspinaErrorHandlerSPtr errorHandler, GeneralSettingsSPtr settings);

      virtual ~FileSaveTool();

      /** \brief Sets the session filename.
       * \param[in] filename file to save the session.
       */
      void setSessionFile(const QString &filename);

      /** \brief Returns the name of the session file.
       *
       */
      const QString fileName() const;

      /** \brief Stores the current undo stack index.
       *
       */
      void updateUndoStackIndex();

    signals:
      void aboutToSaveSession();

      void sessionSaved(const QString &filename);

    protected slots:
      /** \brief Saves the analysis to the given file.
       *
       */
      void save(const QString &fileName);

      /** \brief Saves the session with the name of the session file.
       *
       */
      virtual void onTriggered();

    private slots:
      void autosave();

    protected:
      QTimer                 m_autosave;       /** auto-save timer.                                      */
      QFileInfo              m_sessionFile;    /** session file name.                                    */

    private:
      AnalysisSPtr           m_analysis;       /** application analysis pointer.                         */
      EspinaErrorHandlerSPtr m_errorHandler;   /** application error handler                             */
      GeneralSettingsSPtr    m_settings;       /** application general settings.                         */
      int                    m_undoStackIndex; /** application undo stack index at the time of autosave. */
  };

  //----------------------------------------------------------------------------
  class FileSaveAsTool
  : public FileSaveTool
  {
      Q_OBJECT
    public:
      /** \brief FileSaveAsTool class constructor.
       * \param[in] context application context.
       *
       */
      explicit FileSaveAsTool(Support::Context &context , AnalysisSPtr analysis, EspinaErrorHandlerSPtr errorHandler, GeneralSettingsSPtr settings);

    protected slots:
      virtual void onTriggered() override;
  };

} // namespace ESPINA

#endif // APP_TOOLGROUPS_FILE_FILESAVETOOL_H_
