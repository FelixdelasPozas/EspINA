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
#include <Settings/GeneralSettings/GeneralSettings.h>
#include <Support/Widgets/ProgressTool.h>

namespace ESPINA
{
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
     * \param[in] context application context.
     * \param[in] error handler application error.
     * \param[in] settings application general settings.
     *
     */
    explicit FileOpenTool(Support::Context& context, EspinaErrorHandlerSPtr errorHandler, GeneralSettingsSPtr settings);

    /** \brief Returns the list of files that has been loaded/try to load in the last attempt.
     *
     */
    QStringList files() const;

    /** \brief Returns the analysis.
     *
     */
    AnalysisSPtr analysis();

  signals:
    void analysisLoaded(AnalysisSPtr analysis);

  private slots:
    /** \brief Loads an analysis.
     * \param[in] files list of filenames to load.
     *
     */
    void load(const QStringList &files);

    /** \brief Launches the load task.
     *
     */
    void onTriggered();

  private:
    EspinaErrorHandlerSPtr    m_errorHandler;  /** application error handler */
    QStringList               m_selectedFiles; /** list of file names to be loaded in the task. */
  };
}

#endif // ESPINA_FILE_OPEN_TOOL_H
