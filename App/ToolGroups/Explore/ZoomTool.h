/*

 Copyright (C) 2015 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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

#ifndef ESPINA_ZOOM_TOOL_H_
#define ESPINA_ZOOM_TOOL_H_

// ESPINA
#include <Support/Widgets/Tool.h>
#include <GUI/Types.h>
#include <GUI/View/ViewState.h>
#include <GUI/View/Widgets/Zoom/ZoomEventHandler.h>

using namespace ESPINA::GUI::Representations::Managers;

namespace ESPINA
{
  class RenderView;
  class ZoomWidget;
  
  class ZoomTool
  : public Support::Widgets::ProgressTool
  {
      Q_OBJECT
    public:
      /** \brief ZoomTool class constructor.
       *
       */
      ZoomTool(Support::Context &context);

      /** \brief ZoomTool class virtual destructor.
       *
       */
      virtual ~ZoomTool();

      virtual void abortOperation();

    private slots:
      /** \brief Activates the tool.
       * \param[in] value true to enable, false otherwise.
       *
       */
      void onToolActivated(bool value);

    private:
      GUI::View::ViewState  &m_viewState;
      ZoomEventHandlerSPtr   m_handler;
      TemporalPrototypesSPtr m_factory;
  };

} // namespace ESPINA

#endif // ESPINA_ZOOM_TOOL_H_
