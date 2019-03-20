/*

 Copyright (C) 2019 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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

#ifndef APP_UTILS_UPDATECHECK_H_
#define APP_UTILS_UPDATECHECK_H_

// ESPINA
#include <Core/MultiTasking/Task.h>

// Qt
#include <QDebug>

class QEventLoop;
class QNetworkAccessManager;
class QNetworkReply;

namespace ESPINA
{
  /** \class UpdateCheck
   * \brief Checks the application version against the releases info file on the net.
   *
   */
  class UpdateCheck
  : public Task
  {
      Q_OBJECT
    public:
      static const QString SKIP_VERSION_KEY;    /** Settings key of 'skip version' field.    */
      static const QString LAST_CHECK_TIME_KEY; /** Settings key of 'last check time' field. */

      /** \brief UpdateCheck class constructor.
       * \param[in] scheduler Application task scheduler.
       *
       */
      explicit UpdateCheck(SchedulerSPtr scheduler);

      /** \brief UpdateCheck class virtual destructor.
       *
       */
      virtual ~UpdateCheck();

      /** \brief Returns the version string from the update if newer than the current version. Empty otherwise.
       *
       */
      const QString &versionString() const
      { return m_version; }

      /** \brief Returns the description of the version from the update if newer than the current version. Empty otherwise.
       *
       */
      const QString &versionDescription() const
      { return m_versionDescription; }

      /** \brief Returns true if the task retrieved the update data and exists a newer version.
       *
       */
      const bool hasNewerVersion() const
      { return hasFinished() && !m_version.isEmpty(); }

    protected:
      virtual void run() override;

    private slots:
      /** \brief Fills the data buffer with the contents of the network reply.
       * \param[in] reply Network reply.
       *
       */
      void onDataDownloaded(QNetworkReply *reply);

    private:
      static const QString UPDATE_URL;

      struct VersionNumbers
      {
          unsigned int major; /** major version number. */
          unsigned int minor; /** minor version number. */
          unsigned int patch; /** patch version number. */

          /** \brief VersionNumbers constructor from a string.
           * \param[in] versionString QString containing the version numbers separated by dots.
           *
           */
          VersionNumbers(const QString &versionString);

          /** \brief VersionNumbers constructor from numbers.
           * \param[in] majorNumber Major version number.
           * \param[in] minorNumber Minor version number.
           * \param[in] patchNumber Patch version number.
           *
           */
          VersionNumbers(const unsigned int majorNumber, const unsigned int minorNumber, const unsigned int patchNumber)
          : major{majorNumber}, minor{minorNumber}, patch{patchNumber}
          {};

          /** \brief VersionNumbers constructor. Initialized as invalid.
           *
           */
          VersionNumbers()
          : major{0}, minor{0}, patch{0}
          {};

          /** \brief Returns true if the version is valid.
           *
           */
          const bool isValid() const
          { return !((major == 0) && (minor == 0) && (patch == 0)); };

          /** \brief operator<
           * \param[in] other Other VersionNumbers struct.
           *
           */
          bool operator<(const VersionNumbers &other) const;

          /** \brief operator==
           * \param[in] other Other VersionNumbers struct.
           *
           */
          bool operator==(const VersionNumbers &other) const;

          /** \brief operator>
           * \param[in] other Other VersionNumbers struct.
           *
           */
          bool operator>(const VersionNumbers &other) const;

          /** \brief operator<=
           * \param[in] other Other VersionNumbers struct.
           *
           */
          bool operator<=(const VersionNumbers &other) const;

          /** \brief operator>=
           * \param[in] other Other VersionNumbers struct.
           *
           */
          bool operator>=(const VersionNumbers &other) const;

          /** \brief operator!=
           * \param[in] other Other VersionNumbers struct.
           *
           */
          bool operator!=(const VersionNumbers &other) const;

          /** \brief operator= copy assignment
           * \param[in] other Other VersionNumbers struct.
           *
           */
          VersionNumbers &operator=(const VersionNumbers &other);

          /** \brief Returns the version numbers as a QString.
           *
           */
          const QString toString() const
          { return QString("%1.%2.%3").arg(major).arg(minor).arg(patch); }
      };

      QNetworkAccessManager *m_netManager;         /** net manager.                           */
      QEventLoop            *m_loop;               /** used for waiting.                      */
      QByteArray             m_data;               /** downloaded data or empty if error.     */
      QString                m_error;              /** empty if success or error message.     */
      QString                m_version;            /** newer version or empty.                */
      QString                m_versionDescription; /** description of newer version or empty. */
  };

} // namespace ESPINA

#endif // APP_UTILS_UPDATECHECK_H_
