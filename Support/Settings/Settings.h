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

#ifndef SUPPORT_SETTINGS_SETTINGS_H_
#define SUPPORT_SETTINGS_SETTINGS_H_

#include <Support/EspinaSupport_Export.h>

// Qt
#include <QSettings>
#include <QString>
#include <QDir>

// C++
#include <memory>

#define ESPINA_SETTINGS(settings) QSettings settings("CeSViMa", "ESPINA");

namespace ESPINA
{
  namespace Support
  {
    /** \class ApplicationSettings
     * \brief Application settings.
     *
     */
    class EspinaSupport_EXPORT ApplicationSettings
    {
      public:
        /** \class Frequency
         * \brief Enum class for check frequency.
         *
         */
        enum class UpdateCheckPeriodicity
        {
            ONCE_A_DAY = 0, ONCE_A_WEEK, ONCE_A_MONTH, NEVER
        };

        /** \brief Settings class constructor.
         *
         */
        explicit ApplicationSettings();

        /** \brief Settings class destructor.
         *
         */
        ~ApplicationSettings()
        {}

        /** \brief Sets the user name.
         * \param[in] name user name.
         *
         */
        void setUserName(const QString &name);

        /** \brief Returns user name.
         *
         */
        const QString userName() const
        { return m_userName; }

        /** \brief Enables/disables the loading of any settings file included
         * in the SEG file.
         * \param[in] enable true to enable and false otherwise.
         *
         */
        void setLoadSEGfileSettings(const bool enable);

        /** \brief Returns true if any settings file included in the SEG file must
         *  be loaded.
         *
         */
        const bool loadSEGfileSettings() const
        { return m_loadSEGSettings; }

        /** \brief Sets the path of the temporal storage for the application. Throws exception on error.
         * \param[in] path absolute dir path.
         *
         */
        void setTemporalPath(const QString &path);

        /** \brief Returns the temporal storage path.
         *
         */
        const QString temporalPath() const
        { return m_temporalStoragePath; }

        /** \brief Enables/disables the initial analysis check after loading a file.
         * \param[in] value true to enable the analysis check and false otherwise.
         *
         */
        void setPerformAnalysisCheckOnLoad(const bool value);

        /** \brief Returns the value of the 'analysis check on load' setting.
         *
         */
        const bool performAnalysisCheckOnLoad() const
        { return m_performAnalysisCheck; }

        /** \brief Sets the periodicity of update checks.
         * \param[in] moment UpdateCheckPeriodicity value.
         *
         */
        void setUpdateCheckPeriodicity(const UpdateCheckPeriodicity moment);

        /** \brief Returns the periodicity of update checks.
         *
         */
        const UpdateCheckPeriodicity updateCheckPeriodicity() const
        { return m_updateCheckPeriodicity; }

      private:
        static const QString LOAD_SEG_SETTINGS_KEY;
        static const QString TEMPORAL_STORAGE_PATH_KEY;
        static const QString USER_NAME;
        static const QString PERFORM_ANALYSIS_CHECK;
        static const QString PERFORM_UPDATE_CHECK;
        static const QString CHECK_PERIODICITY_KEY;

        QString                m_userName;               /** user name.                                                                        */
        bool                   m_loadSEGSettings;        /** true to load tool and representation settings from the SEG file, false otherwise. */
        QString                m_temporalStoragePath;    /** path for temporal storate.                                                        */
        bool                   m_performAnalysisCheck;   /** true to perform checks after loading a SEG file, false otherwise.                 */
        UpdateCheckPeriodicity m_updateCheckPeriodicity; /** frequency of update checks.                                                       */
    };

    using GeneralSettingsSPtr = std::shared_ptr<ApplicationSettings>;

  } // namespace Support
} // namespace ESPINA

#endif // SUPPORT_SETTINGS_SETTINGS_H_
