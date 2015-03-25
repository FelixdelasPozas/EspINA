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

#include <Support/Widgets/Tool.h>

// Plugin
#include "AppositionSurfacePlugin_Export.h"
#include <Core/MultiTasking/Task.h>
#include <GUI/Model/ModelAdapter.h>
#include <Core/Extensions/AppositionSurfaceExtension.h>

// ESPINA
#include <Support/ViewManager.h>

class QUndoStack;
class QIcon;
class QObject;
class QString;

namespace ESPINA
{
  class AppositionSurfacePlugin;

  //-----------------------------------------------------------------------------
  class AppositionSurfacePlugin_EXPORT AppositionSurfaceTool
  : public Tool
  {
    Q_OBJECT

  public:
    /** \brief AppositionSurfaceTool class constructor.
     * \param[in] viewManager
     *
     */
    explicit AppositionSurfaceTool(AppositionSurfacePlugin *plugin,
                                   ModelAdapterSPtr         model,
                                   ModelFactorySPtr         factory,
                                   ViewManagerSPtr          viewManager);

    /** \brief AppositionSurfaceTool class virtual destructor.
     *
     */
    virtual ~AppositionSurfaceTool();

    virtual QList<QAction *> actions() const override;

    virtual void abortOperation() override;

    /** \brief Sets the tooltip of the action.
     * \param[in] tooltip tooltip text.
     *
     */
    void setToolTip(const QString &tooltip)
    { m_action->setToolTip(tooltip); }

  private:
    virtual void onToolEnabled(bool enabled);

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
    AppositionSurfacePlugin *m_plugin;
    ModelAdapterSPtr         m_model;
    ModelFactorySPtr         m_factory;
    ViewManagerSPtr          m_viewManager;

    QAction *m_action;
  };

  using SASToolPtr   = AppositionSurfaceTool *;
  using SASToolSPtr  = std::shared_ptr<AppositionSurfaceTool>;

} // namespace ESPINA

#endif// APPOSITION_SURFACE_TOOL_H
