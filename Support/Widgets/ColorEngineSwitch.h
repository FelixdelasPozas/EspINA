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

#ifndef ESPINA_SUPPORT_WIDGETS_COLORENGINESWITCH_H
#define ESPINA_SUPPORT_WIDGETS_COLORENGINESWITCH_H

#include <Support/EspinaSupport_Export.h>

// ESPINA
#include <GUI/Types.h>
#include <Support/Widgets/ProgressTool.h>

namespace ESPINA
{
  namespace Support
  {
    namespace Widgets
    {
      /** \class ColorEngineSwith
       * \brief Generic switch for color engines.
       *
       */
      class EspinaSupport_EXPORT ColorEngineSwitch
      : public ProgressTool
      {
        public:
          /** \brief ColorEngineSwitch class constructor.
           * \param[in] engine color engine object.
           * \param[in] icon text reference for the switch icon (must be included in the application's resource file).
           * \param[in] context application context.
           *
           */
          explicit ColorEngineSwitch(GUI::ColorEngines::ColorEngineSPtr engine, const QString &icon, Context &context);

          /** \brief ColorEngineSwitch class constructor.
           * \param[in] engine color engine object.
           * \param[in] icon switch icon.
           * \param[in] context application context.
           *
           */
          explicit ColorEngineSwitch(GUI::ColorEngines::ColorEngineSPtr engine, const QIcon &icon, Context &context);

          virtual void saveSettings(std::shared_ptr<QSettings> settings);

          virtual void restoreSettings(std::shared_ptr<QSettings> settings);

          /** \brief Returns the switch color engine.
           *
           */
          GUI::ColorEngines::ColorEngineSPtr colorEngine() const;

        private:
          GUI::ColorEngines::ColorEngineSPtr m_engine; /** color engine. */
      };
    }
  }
}

#endif // ESPINA_SUPPORT_WIDGETS_COLORENGINESWITCH_H
