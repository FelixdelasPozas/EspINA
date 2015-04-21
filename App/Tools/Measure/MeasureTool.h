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

#ifndef ESPINA_MEASURE_TOOL_H_
#define ESPINA_MEASURE_TOOL_H_

// ESPINA
#include <GUI/View/ViewState.h>
#include <GUI/View/Widgets/WidgetFactory.h>
#include <Support/Widgets/Tool.h>

class QAction;

namespace ESPINA
{

  class MeasureTool
  : public Tool
  {
    Q_OBJECT

  public:
    /** \brief MeasureTool class constructor.
     * \param[in] viewState
     *
     */
    explicit MeasureTool(GUI::View::ViewState &viewState);

    /** \brief MeasureTool class destructor.
     *
     */
    virtual ~MeasureTool();

    virtual QList<QAction *> actions() const override;

    virtual void abortOperation() override;

  signals:
    void stopMeasuring();

  private slots:
    void onToolActivated(bool value);

    virtual void onToolEnabled(bool enabled) {}

  private:
    using ViewState         = GUI::View::ViewState;
    using WidgetFactorySPtr = GUI::View::Widgets::WidgetFactorySPtr;

    ViewState        &m_viewState;
    EventHandlerSPtr  m_handler;
    WidgetFactorySPtr m_factory;
    QAction          *m_action;
  };

  using MeasureToolPtr  = MeasureTool *;
  using MeasureToolSPtr = std::shared_ptr<MeasureTool>;

} // namespace ESPINA

#endif // ESPINA_MEASURE_TOOL_H_