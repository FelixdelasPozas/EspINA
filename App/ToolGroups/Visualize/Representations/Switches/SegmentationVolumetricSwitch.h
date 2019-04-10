/*
 Copyright (C) 2017  Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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

#ifndef APP_SEGMENTATIONVOLUMETRICSWITCH_H_
#define APP_SEGMENTATIONVOLUMETRICSWITCH_H_

// ESPINA
#include <GUI/Representations/RepresentationManager.h>
#include <GUI/Representations/RepresentationPool.h>
#include <GUI/Types.h>
#include <GUI/Widgets/ToolButton.h>
#include <Support/Representations/RepresentationSwitch.h>

namespace ESPINA
{
  /** \class SegmentationVolumetricSwitch
   * \brief Switch for volumetric representations.
   *
   */
  class SegmentationVolumetricSwitch
  : public RepresentationSwitch
  {
      Q_OBJECT
    public:
      /** \brief SegmentationVolumetricSwitch class constructor.
       * \param[in] gpuManager manager of volumetric gpu representations.
       * \param[in] cpuManager manager of volumetric cpu representations.
       * \param[in] context session context.
       *
       */
      explicit SegmentationVolumetricSwitch(GUI::Representations::RepresentationManagerSPtr gpuManager,
                                            GUI::Representations::RepresentationManagerSPtr cpuManager,
                                            Support::Context                               &context);

      /** \brief SegmentationVolumetricSwitch class virtual destructor.
       *
       */
      virtual ~SegmentationVolumetricSwitch();

      virtual ViewTypeFlags supportedViews() const override;

      virtual void showRepresentations(const GUI::Representations::FrameCSPtr frame) override;

      virtual void hideRepresentations(const GUI::Representations::FrameCSPtr frame) override;

      virtual void restoreSettings(std::shared_ptr<QSettings> settings) override;

      virtual void saveSettings(std::shared_ptr<QSettings> settings) override;

    private slots:
      /** \brief Switches managers on given value.
       * \param[in] enabled true to activate gpu manager and deactivate cpu manager and false otherwise.
       *
       */
      void onModeChanged(bool enabled);

    private:
      /** \brief Helper method to initialize the parameters widgets.
       *
       */
      void initParameterWidgets();

      /** \brief Helper method to connect gpu manager pools.
       *
       */
      void connectGPUManagerPools();

      /** \brief Helper method to disconnect gpu manager pools.
       *
       */
      void disconnectGPUManagerPools();

      /** \brief Helper method to connect cpu manager pools.
       *
       */
      void connectCPUManagerPools();

      /** \brief Helper method to disconnect cpu manager pools.
       *
       */
      void disconnectCPUManagerPools();

      virtual void invalidateRepresentationsImplementation(ViewItemAdapterList items, const GUI::Representations::FrameCSPtr frame) override;

    private:
      GUI::Representations::RepresentationManagerSPtr m_gpuManager;      /** gpu representations' manager. */
      GUI::Representations::RepresentationManagerSPtr m_cpuManager;      /** cpu representations' manager. */
      GUI::Widgets::ToolButton                       *m_gpuOptionButton; /** gpu enabled button.           */
  };

} /* namespace ESPINA */

#endif /* APP_TOOLGROUPS_VISUALIZE_REPRESENTATIONS_SWITCHES_SEGMENTATIONVOLUMETRICSWITCH_H_ */
