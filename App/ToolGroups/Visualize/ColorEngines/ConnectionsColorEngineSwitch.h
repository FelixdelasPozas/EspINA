/*

 Copyright (C) 2018 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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

#ifndef APP_TOOLGROUPS_VISUALIZE_COLORENGINES_CONNECTIONSCOLORENGINESWITCH_H_
#define APP_TOOLGROUPS_VISUALIZE_COLORENGINES_CONNECTIONSCOLORENGINESWITCH_H_

// ESPINA
#include <GUI/Model/ModelAdapter.h>
#include <Support/Widgets/ColorEngineSwitch.h>

class QLabel;

namespace ESPINA
{
  namespace GUI
  {
    namespace ColorEngines
    {
      class ConnectionsColorEngine;
    }

    namespace Widgets
    {
      class ToolButton;
    }
  }

  /** \class ConnectionsColorEngineSwitch
   * \brief Switch for coloring by number of connections.
   *
   */
  class ConnectionsColorEngineSwitch
  : public Support::Widgets::ColorEngineSwitch
  {
      Q_OBJECT

    public:
      /** \brief ConnectionsColorEngineSwitch class constructor.
       * \param[in] context application context.
       *
       */
      explicit ConnectionsColorEngineSwitch(Support::Context& context);

      /** \brief ConnectionsColorEngineSwitch class virtual destructor.
       *
       */
      virtual ~ConnectionsColorEngineSwitch()
      {}

      virtual void restoreSettings(std::shared_ptr<QSettings> settings);

      virtual void saveSettings(std::shared_ptr<QSettings> settings);

    private slots:
      /** \brief Shows the criteria dialog and changes the interface when the user modifies the criteria.
       *
       */
      void onCriteriaButtonPressed(bool value);

    private:
      /** \brief Helper method to create the information widgets of the switch.
       *
       */
      void createWidgets();

      GUI::Widgets::ToolButton *m_criteriaButton;  /** toolbutton for critera dialog.          */
      QLabel                   *m_warning;         /** label to warn if the criteria is empty. */
      QStringList               m_criteria;        /** connection criteria.                    */
      int                       m_validHue;        /** valid color hue.                        */
      int                       m_invalidHue;      /** invalid color hue.                      */
      int                       m_incompleteHue;   /** incomplete color hue.                   */
      int                       m_unconnectedHue;  /** unconnected color hue.                  */
  };
}

#endif // APP_TOOLGROUPS_VISUALIZE_COLORENGINES_CONNECTIONSCOLORENGINESWITCH_H_
