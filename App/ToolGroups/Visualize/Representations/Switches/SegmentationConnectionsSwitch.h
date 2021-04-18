/*

 Copyright (C) 2017 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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

#ifndef APP_TOOLGROUPS_VISUALIZE_REPRESENTATIONS_SWITCHES_SEGMENTATIONCONNECTIONSSWITCH_H_
#define APP_TOOLGROUPS_VISUALIZE_REPRESENTATIONS_SWITCHES_SEGMENTATIONCONNECTIONSSWITCH_H_

// ESPINA
#include <GUI/Representations/Settings/ConnectionPoolSettings.h>
#include <GUI/Widgets/NumericalInput.h>
#include <Support/Representations/BasicRepresentationSwitch.h>

namespace ESPINA
{
  /** \class SegmentationConnectionSwitch
   * \brief Switch for segmentation connections manager.
   *
   */
  class SegmentationConnectionsSwitch
  : public BasicRepresentationSwitch
  {
      Q_OBJECT
    public:
      /** \brief SegmentationConnectionSwith class constructor.
       * \param[in] manager connection's representation manager.
       * \param[in] context application context.
       *
       */
      explicit SegmentationConnectionsSwitch(GUI::Representations::RepresentationManagerSPtr manager,
                                             GUI::Representations::Settings::ConnectionSettingsSPtr settings,
                                             Support::Context &context);

      /** \brief SegmentationConnectionSwith class virtual destructor.
       *
       */
      virtual ~SegmentationConnectionsSwitch()
      {}

      virtual void restoreSettings(std::shared_ptr<QSettings> settings) override;

      virtual void saveSettings(std::shared_ptr<QSettings> settings) override;

    private slots:
      /** \brief Updates the representations scale value.
       * \param[in] value new scale value.
       *
       */
      void onSizeValueChanged(int value);

      /** \brief Updates the widgets when the settings change.
       *
       */
      void onSettingsModified();

    private:
      /** \brief Helper method to initialize and connect parameter widgets.
       *
       */
      void initializeParameterWidgets();

      GUI::Widgets::NumericalInput                          *m_sizeWidget; /** size parameter widget.      */
      GUI::Representations::Settings::ConnectionSettingsSPtr m_settings;   /** connection settings object. */
  };

} // namespace ESPINA

#endif // APP_TOOLGROUPS_VISUALIZE_REPRESENTATIONS_SWITCHES_SEGMENTATIONCONNECTIONSSWITCH_H_
