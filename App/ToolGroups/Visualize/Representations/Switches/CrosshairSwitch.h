/*

 Copyright (C) 2019 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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

#ifndef APP_TOOLGROUPS_VISUALIZE_REPRESENTATIONS_SWITCHES_CROSSHAIRSWITCH_H_
#define APP_TOOLGROUPS_VISUALIZE_REPRESENTATIONS_SWITCHES_CROSSHAIRSWITCH_H_

// ESPINA
#include <Support/Representations/BasicRepresentationSwitch.h>
#include <GUI/Widgets/ToolButton.h>

namespace ESPINA
{
  /** \class CrosshairSwitch
   * \brief Switch for crosshair manager.
   *
   */
  class CrosshairSwitch
  : public BasicRepresentationSwitch
  {
      Q_OBJECT
    public:
      /** \brief CrosshairSwitch class constructor.
       * \param[in] manager Crosshair manager.
       * \param[in] supportedViews Views' flags.
       * \param[in] context Application context.
       *
       */
      explicit CrosshairSwitch(GUI::Representations::RepresentationManagerSPtr manager,
                               ViewTypeFlags supportedViews,
                               Support::Context &context);

      /** \brief CrosshairSwitch class virtual destructor.
       *
       */
      virtual ~CrosshairSwitch()
      {};

      virtual void restoreSettings(std::shared_ptr<QSettings> settings) override;

      virtual void saveSettings(std::shared_ptr<QSettings> settings) override;

      /** \brief Enables/Disables the showing of plane intersections in 3d views.
       * \param[in] value True to enable and false to disable.
       *
       */
      void setShowPlaneIntersections(const bool value);

      /** \brief Returns true if the plane intersections are being shown in 3d views, and false otherwise.
       *
       */
      const bool showPlaneIntersections() const;

    private slots:
      /** \brief Shows/Hides the plane intersections in 3D views.
       *
       */
      void onIntersectionButtonPressed(bool unused);

    private:
      /** \brief Helper method to create and initialize the settings widgets.
       *
       */
      void createSettingsWidgets();

      GUI::Widgets::ToolButton *m_intersectionButton;
  };

} // namespace ESPINA

#endif // APP_TOOLGROUPS_VISUALIZE_REPRESENTATIONS_SWITCHES_CROSSHAIRSWITCH_H_
