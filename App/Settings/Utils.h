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

#ifndef APP_SETTINGS_UTILS_H_
#define APP_SETTINGS_UTILS_H_

// C++
#include <memory>

// Qt
#include <QTemporaryFile>
#include <QSettings>
#include <QDebug>

namespace ESPINA
{
  /** \class TemporalSettings allocates a empty QSettings object.
   *
   */
  class SettingsContainer
  {
    public:
      /** \brief TemporalSettings class constructor.
       *
       */
      SettingsContainer();

      /** \brief Returns a shared pointer for the QSettings object.
       *
       */
      std::shared_ptr<QSettings> settings() const;

      /** \brief Copies the contents of the given settings to the internal settings.
       * \param[i] settings QSettings object to copy keys from.
       *
       */
      void copyFrom(std::shared_ptr<QSettings> settings);

      /** \brief Copies the contents of the settings to the given settings object.
       * \param[i] settings QSettings object to copy keys to.
       *
       */
      void copyTo(std::shared_ptr<QSettings> settings);

    private:
      QTemporaryFile             m_file;
      std::shared_ptr<QSettings> m_settings;
  };

  /** \brief Copies settings from one file to the other settings file.
   * \param[in] from settings to copy from.
   * \param[in] to settings to copy to.
   *
   */
  void copySettings(std::shared_ptr<QSettings> from, std::shared_ptr<QSettings> to);

  /** \brief Convenience method for printing all the values of a QSettings object for debugging.
   * \param[in] debug QDebug stream.
   * \param[in] settings QSettings shared pointer.
   *
   */
  QDebug operator<< (QDebug debug, std::shared_ptr<QSettings> settings);
}

#endif // APP_SETTINGS_UTILS_H_
