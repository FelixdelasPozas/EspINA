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

#ifndef APPOSITIONSURFACETOOLBAR_H
#define APPOSITIONSURFACETOOLBAR_H

// Plugin
#include <Core/MultiTasking/Task.h>
#include <GUI/Model/ModelAdapter.h>
#include "AppositionSurfacePlugin_Export.h"
#include "Core/Extensions/AppositionSurfaceExtension.h"

// ESPINA
#include <Support/ViewManager.h>
#include <Support/Widgets/ToolGroup.h>

class QUndoStack;
class QIcon;
class QObject;
class QString;

namespace ESPINA
{
  class AppositionSurfacePlugin;

  class AppositionSurfaceTool;
  using SASToolPtr   = AppositionSurfaceTool *;
  using SASToolSPtr  = std::shared_ptr<AppositionSurfaceTool>;

  //-----------------------------------------------------------------------------
  class AppositionSurfacePlugin_EXPORT AppositionSurfaceToolGroup
  : public ToolGroup
  {
    Q_OBJECT
  public:
    /* \brief AppositionSurfaceToolGroup class constructor.
     *
     */
    explicit AppositionSurfaceToolGroup(ModelAdapterSPtr model,
                                        QUndoStack *undoStack,
                                        ModelFactorySPtr factory,
                                        ViewManagerSPtr viewManager,
                                        AppositionSurfacePlugin *plugin);

    /* \brief AppositionSurfaceToolGroup class virtual destructor.
     *
     */
    virtual ~AppositionSurfaceToolGroup();

    /* \brief Implements ToolGroup::setEnabled().
     *
     */
    virtual void setEnabled(bool value);

    /* \brief Implements ToolGroup::enabled().
     *
     */
    virtual bool enabled() const;

    /* \brief Implements ToolGroup::tools().
     *
     */
    virtual ToolSList tools();

  public slots:
    /* \brief Changes action enabled/disabled depending on the actual selection.
     *
     */
    void selectionChanged();

    /* \brief Launches the tasks to create SAS from the group of selected segmentations.
     *
     */
    void createSAS();

  private:
    ModelAdapterSPtr         m_model;
    ModelFactorySPtr         m_factory;
    QUndoStack              *m_undoStack;
    SASToolSPtr              m_tool;
    bool                     m_enabled;
    AppositionSurfacePlugin *m_plugin;
  };

  //-----------------------------------------------------------------------------
  class AppositionSurfacePlugin_EXPORT AppositionSurfaceTool
  : public Tool
  {
    Q_OBJECT
    public:
      /* \brief AppositionSurfaceTool class constructor.
       * \param[in] icon, icon for the QAction.
       * \param[in] text, text to use as the QAction tooltip.
       *
       */
      explicit AppositionSurfaceTool(const QIcon& icon, const QString& text);

      /* \brief AppositionSurfaceTool class virtual destructor.
       *
       */
      virtual ~AppositionSurfaceTool();

      /* \brief Implements Tool::setEnabled().
       *
       */
      virtual void setEnabled(bool value)
      { m_action->setEnabled(value); }

      /* \brief Implements Tool::enabled().
       *
       */
      virtual bool enabled() const
      { return m_action->isEnabled(); }

      /* \brief Implements Tool::actions().
       *
       */
      virtual QList<QAction *> actions() const;

      /* \brief Sets the tooltip of the action.
       *
       */
      void setToolTip(const QString &tooltip)
      { m_action->setToolTip(tooltip); }

    signals:
      /* \brief Signal emmited when the QAction has been triggered.
       *
       */
      void triggered();

    private slots:
      /* \brief Emits the triggered signal for the toolgroup.
       *
       */
      void activated()
      { emit triggered(); }

    private:
      QAction *m_action;
  };

} // namespace ESPINA

#endif// APPOSITIONSURFACETOOLBAR_H
