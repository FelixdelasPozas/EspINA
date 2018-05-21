/*

 Copyright (C) 2018 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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

#ifndef GUI_REPRESENTATIONS_SETTINGS_CONNECTIONPOOLSETTINGS_H_
#define GUI_REPRESENTATIONS_SETTINGS_CONNECTIONPOOLSETTINGS_H_

#include <GUI/EspinaGUI_Export.h>

// ESPINA
#include <GUI/Representations/RepresentationPool.h>

namespace ESPINA
{
  namespace GUI
  {
    namespace Representations
    {
      namespace Settings
      {
        /** \class ConnectionPoolSettings
         * \brief Settings for the connection manager.
         *
         */
        class EspinaGUI_EXPORT ConnectionPoolSettings
        : public PoolSettings
        {
          public:
            static const QString CONNECTION_SIZE;

            /** \brief ConnectionPoolSettings class constructor.
             *
             */
            explicit ConnectionPoolSettings();

            /** \brief ConnectionPoolSettings class virtual destructor.
             *
             */
            virtual ~ConnectionPoolSettings()
            {};

            /** \brief Sets the connection size parameter.
             * \param[in] size Connection representation size.
             *
             */
            void setConnectionSize(int size);

            /** \brief Returns the connection size.
             *
             */
            const int connectionSize() const;
        };

        using ConnectionSettingsSPtr = std::shared_ptr<ConnectionPoolSettings>;

      } // namespace Settings
    } // namespace Representations
  } // namespace GUI
} // namespace ESPINA

#endif // GUI_REPRESENTATIONS_SETTINGS_CONNECTIONPOOLSETTINGS_H_
