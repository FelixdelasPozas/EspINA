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

#include <Support/Widgets/ProgressTool.h>

namespace ESPINA {

  class FileOpenTool
  : public Support::Widgets::ProgressTool
  {
    Q_OBJECT

  public:
    explicit FileOpenTool(Support::Context& context);

  signals:
    void analysisLoaded(AnalysisSPtr analysis);

  private slots:
    void onTriggered();

    void onTaskFinished();

  private:
    class LoadTask;

    std::shared_ptr<LoadTask> m_loadTask;
  };
}

#endif // ESPINA_FILE_OPEN_TOOL_H
