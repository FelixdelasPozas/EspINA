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

#ifndef APP_TOOLGROUPS_VISUALIZE_REPRESENTATIONS_SWITCHES_SEGMENTATIONMESHSWITCH_H_
#define APP_TOOLGROUPS_VISUALIZE_REPRESENTATIONS_SWITCHES_SEGMENTATIONMESHSWITCH_H_

// ESPINA
#include <GUI/Representations/RepresentationManager.h>
#include <GUI/Utils/Timer.h>
#include <GUI/View/ViewTypeFlags.h>
#include <GUI/Widgets/NumericalInput.h>
#include <Support/Context.h>
#include <Support/Representations/RepresentationSwitch.h>

namespace ESPINA
{
  class SegmentationMeshPoolSettings;
  
  /** \class SegmentationMeshSwitch
   * \brief Swith for segmentation mesh representations.
   *
   */
  class SegmentationMeshSwitch
  : public RepresentationSwitch
  {
      Q_OBJECT
    public:
      /** \brief SegmentationMeshSwitch class constructor.
       * \param[in] meshManager manager of mesh representations.
       * \param[in] smoothedMeshManager manager of smoothed mesh representations.
       * \param[in] settings representation settings.
       * \param[in] context session context.
       *
       */
      explicit SegmentationMeshSwitch(GUI::Representations::RepresentationManagerSPtr  meshManager,
                                      GUI::Representations::RepresentationManagerSPtr  smoothedMeshManager,
                                      std::shared_ptr<SegmentationMeshPoolSettings>    settings,
                                      Support::Context                                &context);

      /** \brief SegmentationMeshSwitch class virtual destructor.
       *
       */
      virtual ~SegmentationMeshSwitch();

      virtual ViewTypeFlags supportedViews() const override;

      virtual void showRepresentations(const GUI::Representations::FrameCSPtr frame) override;

      virtual void hideRepresentations(const GUI::Representations::FrameCSPtr frame) override;

      virtual void restoreSettings(std::shared_ptr<QSettings> settings) override;

      virtual void saveSettings(std::shared_ptr<QSettings> settings) override;

    private slots:
      /** \brief Propagates changes in the smooth widget to the settings and the representations.
       * \param[in] value integer value [0-100) indicating smoothness factor.
       *
       */
      void onSmoothChanged(int value);

    private:
      /** \brief Initializes the settings widgets of the switch.
       *
       */
      void initWidgets();

      /** \brief Updates the active manager.
       *
       */
      void switchManagers();

      /** \brief Connects the mesh manager pools to the button progress.
       *
       */
      void connectMeshManagersPools();

      /** \brief Disconnects the mesh manager pools from the button progress.
       *
       */
      void disconnectMeshManagersPools();

      /** \brief Helper method to connect the smooth mesh manager's pools to the button progress.
       *
       */
      void connectSmoothMeshManagersPools();

      /** \brief Helper method to disconnect the smooth mesh manager's pools from the button progress.
       *
       */
      void disconnectSmoothMeshManagersPools();

      virtual void invalidateRepresentationsImplementation(ViewItemAdapterList items, const GUI::Representations::FrameCSPtr frame) override;

    private:
      GUI::Representations::RepresentationManagerSPtr m_meshManager;         /** simple mesh representation manager.                     */
      GUI::Representations::RepresentationManagerSPtr m_smoothedMeshManager; /** smoothed mesh representatio manager.                    */
      std::shared_ptr<SegmentationMeshPoolSettings>   m_settings;            /** mesh representation settings.                           */
      GUI::Widgets::NumericalInput                   *m_smooth;              /** smooth value selector widget.                           */
      bool                                            m_smoothEnabled;       /** true if smooth manager is selected and false othrewise. */
  };

} // namespace ESPINA

#endif // APP_TOOLGROUPS_VISUALIZE_REPRESENTATIONS_SWITCHES_SEGMENTATIONMESHSWITCH_H_
