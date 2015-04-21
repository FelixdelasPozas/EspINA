/*

 Copyright (C) 2014 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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

#ifndef ESPINA_SELECTION_MEASURE_TOOL_H
#define ESPINA_SELECTION_MEASURE_TOOL_H

// ESPINA
#include <Support/Widgets/Tool.h>

#include <GUI/View/ViewState.h>
#include <GUI/View/Selection.h>
#include <GUI/View/Widgets/WidgetFactory.h>

namespace ESPINA
{
  class RenderView;
  class SelectionMeasureWidget;

  class SelectionMeasureTool
  : public Tool
  {
    Q_OBJECT
  public:
    /** \brief RulerTool class constructor.
     * \param[in] viewState
     */
    explicit SelectionMeasureTool(GUI::View::ViewState &viewState, SelectionSPtr selection);

    /** \brief RulerTool class destructor.
     *
     */
    virtual ~SelectionMeasureTool();

    virtual QList<QAction *> actions() const override;

    virtual void abortOperation() override;

  private slots:
    void onToolActivated(bool value);

  private:
    virtual void onToolEnabled(bool enabled);

  private:
    using ViewState         = GUI::View::ViewState;
    using WidgetFactorySPtr = GUI::View::Widgets::WidgetFactorySPtr;

    ViewState        &m_viewState;
    WidgetFactorySPtr m_factory;
    QAction          *m_action;
  };
} // namespace ESPINA

#endif // ESPINA_SELECTION_MEASURE_TOOL_H
