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

#ifndef APP_TOOLGROUPS_VISUALIZE_REPRESENTATIONS_SWITCHES_SEGMENTATIONCONTOURSWITCH_H_
#define APP_TOOLGROUPS_VISUALIZE_REPRESENTATIONS_SWITCHES_SEGMENTATIONCONTOURSWITCH_H_

// ESPINA
#include <GUI/Representations/RepresentationManager.h>
#include <GUI/Representations/Settings/SegmentationContourPoolSettings.h>
#include <GUI/Utils/Timer.h>
#include <GUI/View/ViewTypeFlags.h>
#include <GUI/Widgets/NumericalInput.h>
#include <Support/Context.h>
#include <Support/Representations/BasicRepresentationSwitch.h>

class QComboBox;

namespace ESPINA
{
  class SegmentationContourSwitch
  : public BasicRepresentationSwitch
  {
      Q_OBJECT
    public:
      /** \brief SegmentationContourSwitch class constructor.
       * \param[in] manager manager to be handled by this switch.
       * \param[in] settings settings of the representation of the manager.
       * \param[in] supportedViews flags of the supported views of the manager.
       * \param[in] context session context.
       *
       */
      explicit SegmentationContourSwitch(GUI::Representations::RepresentationManagerSPtr  manager,
                                         std::shared_ptr<SegmentationContourPoolSettings> settings,
                                         ViewTypeFlags                                    supportedViews,
                                         Support::Context                                &context);

      /** \brief SegmentationContour class virtual destructor.
       *
       */
      virtual ~SegmentationContourSwitch();

      virtual void restoreSettings(std::shared_ptr<QSettings> settings) override;

      virtual void saveSettings(std::shared_ptr<QSettings> settings) override;

    private slots:
      /** \brief Propagates changes in the opacity widget to the settings and the representations.
       *
       */
      void onOpacityChanged();

      /** \brief Propagates changes in the width widget to the settings and the representations.
       *
       */
      void onWidthChanged();

      /** \brief Propagates changes in the pattern widget to the settings and the representations.
       *
       */
      void onPatternChanged();

      /** \brief Propagates changes in the settings to the widgets.
       *
       */
      void onSettingsModified();

    private:
      /** \brief Initializes the settings widgets of the switch.
       *
       */
      void initWidgets();

      std::shared_ptr<SegmentationContourPoolSettings> m_settings;
      GUI::Widgets::NumericalInput                    *m_opacityWidget;
      QComboBox                                       *m_width;
      QComboBox                                       *m_pattern;
  };

} // namespace ESPINA

#endif // APP_TOOLGROUPS_VISUALIZE_REPRESENTATIONS_SWITCHES_SEGMENTATIONCONTOURSWITCH_H_
