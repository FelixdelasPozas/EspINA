/*
 *
 * Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 * This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
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

#ifndef ESPINA_RESET_ZOOM_TOOL_H
#define ESPINA_RESET_ZOOM_TOOL_H

// ESPINA
#include <Support/Widgets/Tool.h>
#include <GUI/View/ViewState.h>

class QAction;

namespace ESPINA
{

  class ResetZoom
  : public Support::Widgets::ProgressTool
  {
    Q_OBJECT
  public:
    /** \brief ResetZoom class constructor.
     */
    explicit ResetZoom(Support::Context &context);

    /** \brief ResetZoom class destructor.
     *
     */
    virtual ~ResetZoom();

  private slots:
    /** \brief Slot to activate when the action gets triggered. Resets the views
     * via ViewManager.
     *
     */
    void resetViews();

  private:
    GUI::View::ViewState &m_viewState;
  };

  using ResetZoomSPtr = std::shared_ptr<ResetZoom>;

} // namespace ESPINA

#endif // ESPINA_SEED_GROW_SEGMENTATION_TOOL_H
