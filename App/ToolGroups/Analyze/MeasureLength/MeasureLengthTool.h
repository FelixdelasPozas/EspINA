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
#include <Support/Widgets/ProgressTool.h>
#include <GUI/View/Widgets/Measures/MeasureEventHandler.h>

class QAction;

using namespace ESPINA::GUI::View::Widgets::Measures;

namespace ESPINA
{
  class MeasureLengthTool
  : public Support::Widgets::ProgressTool
  {
    Q_OBJECT

  public:
    /** \brief MeasureLengthTool class constructor.
     * \param[in] viewState
     *
     */
    explicit MeasureLengthTool(Support::Context &context);

    /** \brief MeasureLengthTool class destructor.
     *
     */
    virtual ~MeasureLengthTool();

    virtual void abortOperation();

  signals:
    void stopMeasuring();

  private slots:
    virtual void onToolEnabled(bool enabled) {}

    void onToolActivated(bool value);

  private:
    using ViewState              = GUI::View::ViewState;
    using TemporalPrototypesSPtr = GUI::Representations::Managers::TemporalPrototypesSPtr;

    ViewState              &m_viewState;
    MeasureEventHandlerSPtr m_handler;
    TemporalPrototypesSPtr  m_prototypes;
  };

  using MeasureToolPtr  = MeasureLengthTool *;
  using MeasureToolSPtr = std::shared_ptr<MeasureLengthTool>;

} // namespace ESPINA

#endif // ESPINA_MEASURE_TOOL_H_
