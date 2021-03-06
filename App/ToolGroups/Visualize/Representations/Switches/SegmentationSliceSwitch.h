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

#ifndef APP_TOOLGROUPS_VISUALIZE_REPRESENTATIONS_SWITCHES_SEGMENTATIONSLICESWITCH_H_
#define APP_TOOLGROUPS_VISUALIZE_REPRESENTATIONS_SWITCHES_SEGMENTATIONSLICESWITCH_H_

// ESPINA
#include <GUI/Representations/RepresentationManager.h>
#include <GUI/Representations/Settings/SegmentationSlicePoolSettings.h>
#include <GUI/View/ViewTypeFlags.h>
#include <GUI/Widgets/NumericalInput.h>
#include <Support/Context.h>
#include <Support/Representations/BasicRepresentationSwitch.h>

namespace ESPINA
{
  /** \class SegmentationSliceSwitch
   * \brief Switch for segmentation slice representation.
   *
   */
  class SegmentationSliceSwitch
  : public BasicRepresentationSwitch
  {
      Q_OBJECT
    public:
      /** \brief SegmentationSliceSwitch class constructor.
       * \param[in] id tool id.
       * \param[in] manager manager to be handled by this switch.
       * \param[in] settings settings of the representation of the manager.
       * \param[in] supportedViews flags of the supported views of the manager.
       * \param[in] timer view states' timer.
       * \param[in] context session context.
       *
       */
      explicit SegmentationSliceSwitch(const QString &id,
                                       GUI::Representations::RepresentationManagerSPtr manager,
                                       std::shared_ptr<SegmentationSlicePoolSettings>  settings,
                                       ViewTypeFlags                                   supportedViews,
                                       Support::Context                               &context);

      /** \brief SegmentationSliceSwitch class virtual destructor.
       *
       */
      virtual ~SegmentationSliceSwitch();

      virtual void restoreSettings(std::shared_ptr<QSettings> settings) override;

      virtual void saveSettings(std::shared_ptr<QSettings> settings) override;

    private slots:
      /** \brief Propagates changes in the opacity widget to the settings and the representations.
       *
       */
      void onOpacityChanged();

      /** \brief Propagates changes in the settings to the widgets.
       *
       */
      void onSettingsModified();

    private:
      /** \brief Initializes the settings widgets of the switch.
       *
       */
      void initWidgets();

      std::shared_ptr<SegmentationSlicePoolSettings> m_settings;      /** slice representation settings. */
      GUI::Widgets::NumericalInput                  *m_opacityWidget; /** opacity selector widget.       */
  };

} // namespace ESPINA

#endif // APP_TOOLGROUPS_VISUALIZE_REPRESENTATIONS_SWITCHES_SEGMENTATIONSLICESWITCH_H_
