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

#ifndef APP_TOOLGROUPS_VISUALIZE_DAYNIGHTTOOL_H_
#define APP_TOOLGROUPS_VISUALIZE_DAYNIGHTTOOL_H_

// ESPINA
#include <ToolGroups/Visualize/GenericTogglableTool.h>

namespace ESPINA
{
  class EspinaMainWindow;

  /** \class FullscreenTool
   * \brief Implements the button for enabling/disabling the dark theme.
   *
   */
  class DayNightTool
  : public GenericTogglableTool
  {
      Q_OBJECT
    public:
      /** \brief DayNightTool class constructor.
       * \param[in] context application context.
       *
       */
      explicit DayNightTool(Support::Context &context);

      /** \brief DayNightTool class virtual destructor.
       *
       */
      virtual ~DayNightTool()
      {}

    private slots:
      /** \brief Set the visual theme to light or dark depending on the value.
       * \param[in] value true to set dark mode and false to set light mode.
       *
       */
      void onToggled(bool value);
  };

} // namespace ESPINA

#endif // APP_TOOLGROUPS_VISUALIZE_DAYNIGHTTOOL_H_
