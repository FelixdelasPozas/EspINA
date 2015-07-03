/*

    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>

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


#ifndef GENERALSETTINGS_H
#define GENERALSETTINGS_H

// Qt
#include <QObject>
#include <QString>
#include <QDir>

// C++
#include <memory>

class QSettings;

namespace ESPINA
{
  class GeneralSettings
  {
  public:
    static const QString LOAD_SEG_SETTINGS_KEY;

    /** \brief GeneralSettings class constructor.
     *
     */
    explicit GeneralSettings();

    /** \brief GeneralSettings class destructor.
     *
     */
    ~GeneralSettings();

    /** \brief Returns user name.
     *
     */
    QString userName() const
    {return m_userName;}

    /** \brief Sets the user name.
     * \param[in] name user name.
     *
     */
    void setUserName(const QString &name);

    /** \brief Returns auto-save interval time in minutes.
     *
     */
    int autosaveInterval() const
    {return m_autosaveInterval;}

    /** \brief Sets the auto-save interval.
     * \param[in] min minutes value.
     *
     */
    void setAutosaveInterval(int min);

    /** \brief Returns the path of the auto-save file.
     *
     */
    QDir autosavePath() const
    {return m_autosavePath;}

    /** \brief Sets the auto-save path.
     *
     */
    void setAutosavePath(const QString &path);

    /** \brief Enables/disables the loading of any settings file included
     * in the SEG file.
     * \param[in] enable true to enable and false otherwise.
     *
     */
    void setLoadSEGfileSettings(bool enable);

    /** \brief Returns true if any settings file included in the SEG file must
     *  be loaded.
     *
     */
    bool loadSEGfileSettings() const
    { return m_loadSEGSettings; }

  private:
    QString m_userName;
    int     m_autosaveInterval;
    QDir    m_autosavePath;
    bool    m_loadSEGSettings;
  };

  using GeneralSettingsSPtr = std::shared_ptr<GeneralSettings>;

} // namespace ESPINA

#endif // GENERALSETTINGS_H
