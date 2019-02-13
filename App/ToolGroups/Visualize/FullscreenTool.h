/*

 Copyright (C) 2015 Félix de las Pozas Álvarez <fpozas@cesvima.upm.es>

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

#ifndef APP_TOOLGROUPS_VISUALIZE_FULLSCREENTOOL_H_
#define APP_TOOLGROUPS_VISUALIZE_FULLSCREENTOOL_H_

// ESPINA
#include <ToolGroups/Visualize/GenericTogglableTool.h>

namespace ESPINA
{
  class EspinaMainWindow;

  /** \class FullscreenTool
   * \brief Implements the button for the fullscreen action for the main window.
   *
   */
  class FullscreenTool
  : public GenericTogglableTool
  {
      Q_OBJECT
    public:
      /** \brief FullscreenTool class constructor.
       * \param[in] context application context.
       * \param[in] window pointer to main window.
       *
       */
      explicit FullscreenTool(Support::Context &context, EspinaMainWindow &window);

      /** \brief FullscreenTool class virtual destructor.
       *
       */
      virtual ~FullscreenTool()
      {}

    private slots:
      /** \brief Shows the main windows fullscreen or normal depending on the value.
       * \param[in] value true to enter fullscreen and false otherwise.
       *
       */
      void onToggled(bool value);

    private:
      EspinaMainWindow &m_window;

  };

} // namespace ESPINA

#endif // APP_TOOLGROUPS_VISUALIZE_FULLSCREENTOOL_H_
