/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2015  Jorge Peña Pastor <jpena@cesvima.upm.es>
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

#ifndef ESPINA_FILE_OPEN_TOOL_H
#define ESPINA_FILE_OPEN_TOOL_H

// ESPINA
#include <App/EspinaErrorHandler.h>
#include <Support/Widgets/ProgressTool.h>

namespace ESPINA
{
  class EspinaMainWindow;

  /** \class FileOpenTool
   * \brief Class to load session files in a background task.
   *
   */
  class FileOpenTool
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
    explicit FileOpenTool(const QString& id,
                          const QString& icon,
                          const QString& tooltip,
                          Support::Context& context,
                          EspinaErrorHandlerSPtr handler);

    /** \brief FileOpenTool class virtual destructor.
     *
     */
    virtual ~FileOpenTool();

    /** \brief Returns the list of files that has been loaded/try to load in the last attempt.
     *
     */
    QStringList loadedFiles() const;

    /** \brief Sets the application pointer for the close session callback when opening a new session.
     * \param[in] callback application main window pointer.
     *
     */
    void setCloseCallback(EspinaMainWindow *callback);

  public slots:
    /** \brief Load requested file
     * \param[in] file to load.
     *
     */
    void loadAnalysis(const QString &file);

    /** \brief Load requested files
     * \param[in] files to load.
     *
     */
    void load(const QStringList &files);

  signals:
    void analysisLoaded(AnalysisSPtr analysis);

  private slots:
    /** \brief Launches the load task.
     *
     */
    void onTriggered();

  private:
    /** \brief Helper method that returns true if the given filename is the current autosave file.
     * \param[in] fileName Absolute file path.
     *
     */
    const bool isAutoSaveFile(const QString &fileName);

  private:
    EspinaErrorHandlerSPtr m_errorHandler;  /** application error handler                                                    */
    QStringList            m_loadedFiles;   /** list of file names to be loaded in the task.                                 */
    EspinaMainWindow      *m_closeCallback; /** application pointer, used to close current session before loading a new one. */
  };
}

#endif // ESPINA_FILE_OPEN_TOOL_H
