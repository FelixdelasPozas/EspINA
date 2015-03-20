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
#include <GUI/View/Widgets/EspinaWidget.h>
#include <Support/Widgets/Tool.h>
#include <Support/ViewManager.h>

class QAction;

namespace ESPINA
{

  class MeasureTool
  : public Tool
  {
    Q_OBJECT
  public:
    /** \brief MeasureTool class constructor.
     * \param[in] viewManager view manager smart pointer.
     *
     */
    explicit MeasureTool(ViewManagerSPtr viewManager);

    /** \brief MeasureTool class destructor.
     *
     */
    virtual ~MeasureTool();

    virtual QList<QAction *> actions() const override;

    virtual void abortOperation() override;

  public slots:
    /** \brief Initializes/De-initializes tool.
     * \param[in] value true to initialize tool, false to de-initialize.
     */
    void initTool(bool value);

  signals:
    void stopMeasuring();

  private:
    virtual void onToolEnabled(bool enabled);

  private:
    EspinaWidgetSPtr m_widget;
    EventHandlerSPtr m_handler;
    ViewManagerSPtr  m_viewManager;
    QAction         *m_action;
  };

  using MeasureToolPtr  = MeasureTool *;
  using MeasureToolSPtr = std::shared_ptr<MeasureTool>;

} // namespace ESPINA

#endif /* MEASURETOOL_H_ */
