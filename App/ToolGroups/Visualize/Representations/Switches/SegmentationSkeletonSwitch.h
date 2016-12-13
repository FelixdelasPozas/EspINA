/*

 Copyright (C) 2016 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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

#ifndef APP_TOOLGROUPS_VISUALIZE_REPRESENTATIONS_SWITCHES_SEGMENTATIONSKELETONSWITCH_H_
#define APP_TOOLGROUPS_VISUALIZE_REPRESENTATIONS_SWITCHES_SEGMENTATIONSKELETONSWITCH_H_

// ESPINA
#include <GUI/Representations/RepresentationManager.h>
#include <GUI/Representations/Settings/SegmentationSkeletonPoolSettings.h>
#include <GUI/View/ViewTypeFlags.h>
#include <GUI/Widgets/NumericalInput.h>
#include <GUI/Widgets/ToolButton.h>
#include <Support/Context.h>
#include <Support/Representations/BasicRepresentationSwitch.h>

// Qt
#include <QSettings>

class QComboBox;

namespace ESPINA
{
  /** \class SegmentationSkeletonSwitch
   * \brief Swith for enabling/disabling the skeleton representation and managing it's settings.
   *
   */
  class SegmentationSkeletonSwitch
  : public BasicRepresentationSwitch
  {
      Q_OBJECT
    public:
      /** \brief SegmentationSkeletonSwitch class constructor.
       * \param[in] id switch id.
       * \param[in] manager manager to be handled by this switch.
       * \param[in] settings settings of the representation of the manager.
       * \param[in] supportedViews flags of the supported views of the manager.
       * \param[in] context session context.
       *
       */
      explicit SegmentationSkeletonSwitch(const QString                                                          &id,
                                          GUI::Representations::RepresentationManagerSPtr                         manager,
                                          std::shared_ptr<GUI::Representations::SegmentationSkeletonPoolSettings> settings,
                                          ViewTypeFlags                                                           supportedViews,
                                          Support::Context                                                       &context);

      /** \brief SegmentationSkeletonSwitch class virtual destructor.
       *
       */
      virtual ~SegmentationSkeletonSwitch();

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

      /** \brief Propagates changes in the annotations visibility widget to the settings and the representations.
       *
       */
      void onAnnotationsVisibilityChanged();

      /** \brief Propagates changes in the settings to the widgets.
       *
       */
      void onSettingsModified();

    private:
      /** \brief Initializes the settings widgets of the switch.
       *
       */
      void initWidgets();

      std::shared_ptr<GUI::Representations::SegmentationSkeletonPoolSettings> m_settings; /** settings object. */

      GUI::Widgets::NumericalInput *m_opacityWidget;      /** representation's opacity widget.                */
      QComboBox                    *m_widthWidget;        /** representation's width widget.                  */
      GUI::Widgets::ToolButton     *m_annotationsWidget;  /** representation's annotations visibility widget. */
  };

} // namespace ESPINA

#endif // APP_TOOLGROUPS_VISUALIZE_REPRESENTATIONS_SWITCHES_SEGMENTATIONSKELETONSWITCH_H_
