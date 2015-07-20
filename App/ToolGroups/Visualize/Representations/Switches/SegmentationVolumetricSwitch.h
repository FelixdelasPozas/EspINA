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

#ifndef APP_TOOLGROUPS_VISUALIZE_REPRESENTATIONS_SWITCHES_SEGMENTATIONVOLUMETRICSWITCH_H_
#define APP_TOOLGROUPS_VISUALIZE_REPRESENTATIONS_SWITCHES_SEGMENTATIONVOLUMETRICSWITCH_H_

// ESPINA
#include <GUI/Representations/RepresentationManager.h>
#include <GUI/Utils/Timer.h>
#include <GUI/View/ViewTypeFlags.h>
#include <Support/Context.h>
#include <Support/Representations/RepresentationSwitch.h>

class QPushButton;

namespace ESPINA
{
  
  class SegmentationVolumetricSwitch
  : public RepresentationSwitch
  {
      Q_OBJECT
    public:
      /** \brief SegmentationVolumetricSwitch class constructor.
       * \param[in] cpuManager manager of volumetric cpu representations.
       * \param[in] gpuManager manager of volumetric gpu representations.
       * \param[in] supportedViews flags of the supported views of the manager.
       * \param[in] timer view states' timer.
       * \param[in] context session context.
       *
       */
      explicit SegmentationVolumetricSwitch(GUI::Representations::RepresentationManagerSPtr  cpuManager,
                                            GUI::Representations::RepresentationManagerSPtr  gpuManager,
                                            ViewTypeFlags                                    supportedViews,
                                            Timer                                           &timer,
                                            Support::Context                                &context);

      /** \brief SegmentationVolumetricSwitch class virtual destructor.
       *
       */
      virtual ~SegmentationVolumetricSwitch();

      virtual ViewTypeFlags supportedViews();

      virtual void showRepresentations(TimeStamp t) override;

      virtual void hideRepresentations(TimeStamp t) override;

      virtual void restoreSettings(std::shared_ptr<QSettings> settings) override;

      virtual void saveSettings(std::shared_ptr<QSettings> settings) override;

    private slots:
      /** \brief Propagates changes in the gpu widget to the settings and the representations.
       * \param[in] check sender button check status.
       *
       */
      void onModeChanged(bool check);

    private:
      /** \brief Initializes the settings widgets of the switch.
       *
       */
      void initWidgets();

      virtual void invalidateRepresentationsImplementation(ViewItemAdapterList items, TimeStamp t) override;

    private:
      GUI::Representations::RepresentationManagerSPtr m_cpuManager;
      GUI::Representations::RepresentationManagerSPtr m_gpuManager;
      ViewTypeFlags                                   m_flags;
      QPushButton                                    *m_gpuEnable;
  };

} // namespace ESPINA

#endif // APP_TOOLGROUPS_VISUALIZE_REPRESENTATIONS_SWITCHES_SEGMENTATIONVOLUMETRICSWITCH_H_
