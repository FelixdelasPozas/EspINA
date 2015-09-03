/*
 *
 *    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 *    This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef APPOSITION_SURFACE_TOOL_H
#define APPOSITION_SURFACE_TOOL_H

#include <Support/Widgets/ProgressTool.h>

// Plugin
#include "AppositionSurfacePlugin_Export.h"
#include <Core/MultiTasking/Task.h>
#include <GUI/Model/ModelAdapter.h>
#include <Core/Extensions/AppositionSurfaceExtension.h>

// ESPINA
#include <Support/Context.h>
#include <QAction>

class QUndoStack;
class QIcon;
class QObject;
class QString;

namespace ESPINA
{
  class AppositionSurfacePlugin;

  //-----------------------------------------------------------------------------
  class AppositionSurfacePlugin_EXPORT AppositionSurfaceTool
  : public Support::Widgets::ProgressTool
  {
    Q_OBJECT

  public:
    /** \brief AppositionSurfaceTool class constructor.
     * \param[in] viewManager
     *
     */
    explicit AppositionSurfaceTool(AppositionSurfacePlugin *plugin, Support::Context &context);

    /** \brief AppositionSurfaceTool class virtual destructor.
     *
     */
    virtual ~AppositionSurfaceTool();

  private slots:
    /** \brief Changes action enabled/disabled depending on the actual selection.
     *
     */
    void selectionChanged();

    /** \brief Launches the tasks to create SAS from the group of selected segmentations.
     *
     */
    void createSAS();

  private:
    QString defaultTooltip() const;

  private:
    AppositionSurfacePlugin *m_plugin;
  };

  using SASToolPtr   = AppositionSurfaceTool *;
  using SASToolSPtr  = std::shared_ptr<AppositionSurfaceTool>;

} // namespace ESPINA

#endif// APPOSITION_SURFACE_TOOL_H
