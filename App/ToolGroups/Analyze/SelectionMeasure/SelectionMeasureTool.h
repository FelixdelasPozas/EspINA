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
#include <Support/Widgets/ProgressTool.h>

#include <GUI/View/ViewState.h>
#include <GUI/View/Selection.h>

namespace ESPINA
{
  class RenderView;
  class SelectionMeasureWidget;

  /** \class SelectionMeasureTool
   * \brief Tool button for selection measure tool.
   *
   */
  class SelectionMeasureTool
  : public Support::Widgets::ProgressTool
  {
    Q_OBJECT
  public:
    /** \brief RulerTool class constructor.
     * \param[in] viewState
     */
    explicit SelectionMeasureTool(Support::Context &context);

    /** \brief RulerTool class destructor.
     *
     */
    virtual ~SelectionMeasureTool();

  private slots:
    /** \brief Shows/hides the tools representations.
     * \param[in] value true to show the representations and false otherwise.
     *
     */
    void onToolActivated(bool value);

  private:
    using ViewState              = GUI::View::ViewState;
    using TemporalPrototypesSPtr = GUI::Representations::Managers::TemporalPrototypesSPtr;

    ViewState             &m_viewState; /** application's view state reference. */
    TemporalPrototypesSPtr m_factory;   /** tool's representations factory.      */
  };
} // namespace ESPINA

#endif // ESPINA_SELECTION_MEASURE_TOOL_H
