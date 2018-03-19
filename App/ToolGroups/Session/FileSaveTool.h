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

#ifndef ESPINA_FILE_SAVE_TOOL_H
#define ESPINA_FILE_SAVE_TOOL_H

// ESPINA
#include <Support/Widgets/ProgressTool.h>
#include "EspinaErrorHandler.h"

namespace ESPINA
{
  namespace IO
  {
    namespace SegFile
    {
      class SaveThread;
    }
  }

  /** \class FileSaveTool
   * \brief Tool that handles file save.
   *
   */
  class FileSaveTool
  : public Support::Widgets::ProgressTool
  {
      Q_OBJECT
    public:
      /** \brief FileOpenTool class constructor.
       *
       * \param[in] id of the tool
       * \param[in] icon for the tool
       * \param[in] tooltip to be display on mouse hover
       * \param[in] context application context
       * \param[in] handler error handler
       *
       */
      explicit FileSaveTool(const QString         &id,
                            const QString         &icon,
                            const QString         &tooltip,
                            Support::Context      &context,
                            AnalysisSPtr          &analysis,
                            EspinaErrorHandlerSPtr handler);

      /** \brief FileSaveTool class virtual destructor.
       *
       */
      virtual ~FileSaveTool();

      /** \brief Sets the filename to save analysis to
       * \param[in] filename file to save current analysis
       *
       * If no filename is set, then a SaveDialog is displayed
       * to select the filename
       */
      void setSaveFilename(const QString &filename);

      /** \brief Returns the filename to save analysis to
       *
       */
      const QString saveFilename() const;

      /** \brief Stores the current undo stack index.
       *
       */
      void updateUndoStackIndex();

      /** \brief Sets the behavior of the tool regarding asking the user.
       * \param[in] value true to always ask for the file name, and false otherwise.
       *
       */
      void setAlwaysAskUser(bool value);

    public slots:
      void saveAnalysis();

      /** \brief Saves the analysis to the given file.
       *
       */
      bool saveAnalysis(const QString &fileName);

      /** \brief Handles the save thread result.
       *
       */
      void onSaveThreadFinished();

      /** \brief Aborts the running task and frees it. Connected from the main window.
       *
       */
      void abortTask();

    signals:
      void aboutToSaveSession();

      void sessionSaved(const QString &filename, bool success);

    protected:
      QString m_filename;

    private:
      /** \brief Returns the icon for the tool during the save process.
       *
       */
      QIcon saveIcon() const;

      /** \brief Returns the default tool icon.
       *
       */
      QIcon defaultIcon() const;

      AnalysisSPtr          &m_analysis;     /** analysis to save.                                         */
      EspinaErrorHandlerSPtr m_errorHandler; /** application error handler.                                */
      bool                   m_askAlways;    /** true to ask to user in case of error and false otherwise. */
      const QIcon            m_icon;         /** tool icon.                                                */

      std::shared_ptr<IO::SegFile::SaveThread> m_thread; /** saving thread. */
  };
} // namespace ESPINA

#endif // ESPINA_FILE_SAVE_TOOL_H
