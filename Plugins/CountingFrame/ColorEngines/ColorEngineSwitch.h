/*
 *    Copyright (C) 2015  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 *    This file is part of ESPINA.
 *
 *    ESPINA is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ESPINA_CF_COLOR_ENGINESWITCH_H
#define ESPINA_CF_COLOR_ENGINESWITCH_H

// ESPINA
#include <Support/Widgets/ColorEngineSwitch.h>
#include "ColorEngine.h"

namespace ESPINA
{
  namespace CF
  {
    class CountingFrameManager;

    /** \class ColorEngineSwitch
     * \brief Tool button for the counting frame coloring engine.
     *
     */
    class CountingFramePlugin_EXPORT ColorEngineSwitch
    : public Support::Widgets::ColorEngineSwitch
    {
      Q_OBJECT
    public:
      /** \brief ColorEngineSwitch class constructor.
       * \param[in] manager plugin's counting frame manager.
       * \param[in] engine color engine.
       * \param[in] context application context.
       *
       */
      explicit ColorEngineSwitch(CountingFrameManager *manager, CountingFrameColorEngineSPtr engine, Support::Context& context);

      virtual void saveSettings(std::shared_ptr<QSettings> settings) override;

      virtual void restoreSettings(std::shared_ptr<QSettings> settings) override;

    private:
      /** \brief Helper method to initialize the widgets
       *
       */
      void initWidgets();

    private slots:
      /** \brief Updates the opacity of the representations.
       *
       */
      void onOpacityChanged(int value);

      /** \brief Enables/disables the tool depending on the number of counting frames.
       *
       */
      void onCountingFrameNumberModified();

    private:
      CountingFrameColorEngineSPtr m_engine;  /** counting frame color engine.     */
      CountingFrameManager        *m_manager; /** plugin's counting frame manager. */
    };
  }
}

#endif // ESPINA_CF_COLOR_ENGINESWITCH_H
