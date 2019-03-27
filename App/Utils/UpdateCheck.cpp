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

// ESPINA
#include <EspinaConfig.h>
#include <Utils/UpdateCheck.h>
#include <Support/Settings/Settings.h>

// C++
#include <unistd.h>

// Qt
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkAccessManager>
#include <QXmlStreamReader>
#include <QThread>
#include <QEventLoop>
#include <QTimer>
#include <QDateTime>

using namespace ESPINA;
using namespace ESPINA::Support;

const QString UpdateCheck::UPDATE_URL = "http://cajalbbp.cesvima.upm.es/espina/packages/releases.xml";

const QString UpdateCheck::SKIP_VERSION_KEY    = "Skip version";
const QString UpdateCheck::LAST_CHECK_TIME_KEY = "Last update check time";

const QString RELEASE_ELEMENT = "Release";
const QString DETAILS_ELEMENT = "Description";
const QString VERSION_ELEMENT = "Version";

//--------------------------------------------------------------------
UpdateCheck::UpdateCheck(SchedulerSPtr scheduler)
: Task        {scheduler}
, m_netManager{nullptr}
, m_loop      {nullptr}
{
  setDescription("Looking for updates.");
}

//--------------------------------------------------------------------
UpdateCheck::~UpdateCheck()
{
  if(m_netManager) delete m_netManager;
  if(m_loop)       delete m_loop;
}

//--------------------------------------------------------------------
void UpdateCheck::run()
{
  const VersionNumbers ESPINAVERSION{ESPINA_VERSION};
  if(!ESPINAVERSION.isValid()) return;

  ApplicationSettings espinaSettings;

  ESPINA_SETTINGS(settings);
  const VersionNumbers SKIP_VERSION{settings.value(SKIP_VERSION_KEY, QString()).toString()};
  const auto LAST_CHECK  = settings.value(LAST_CHECK_TIME_KEY, QDateTime()).toDateTime();
  const auto PERIODICITY = espinaSettings.updateCheckPeriodicity();
  const auto now         = QDateTime::currentDateTime();

  switch(PERIODICITY)
  {
    case ApplicationSettings::UpdateCheckPeriodicity::NEVER:
      return;
      break;
    case ApplicationSettings::UpdateCheckPeriodicity::ONCE_A_MONTH:
      if(LAST_CHECK.isValid() && LAST_CHECK.addMonths(1) > now) return;
      break;
    case ApplicationSettings::UpdateCheckPeriodicity::ONCE_A_WEEK:
      if(LAST_CHECK.isValid() && LAST_CHECK.addDays(7) > now) return;
      break;
    default:
    case ApplicationSettings::UpdateCheckPeriodicity::ONCE_A_DAY:
      if(LAST_CHECK.isValid() && LAST_CHECK.addDays(1) > now) return;
      break;
  }

  settings.setValue(LAST_CHECK_TIME_KEY, now);
  settings.sync();

  m_netManager = new QNetworkAccessManager(this);
  m_loop = new QEventLoop{this};

  connect(m_netManager, SIGNAL (finished(QNetworkReply*)), this, SLOT (onDataDownloaded(QNetworkReply*)));

  QNetworkRequest request(UPDATE_URL);
  m_netManager->get(request);

  reportProgress(25);

  QTimer timer;
  timer.setSingleShot(true);
  connect(&timer, SIGNAL(timeout()), m_loop, SLOT(quit()));
  timer.start(30000);
  m_loop->exec();

  disconnect(&timer, SIGNAL(timeout()), m_loop, SLOT(quit()));
  if(!timer.isActive())
  {
    m_error = tr("Timeout.");
  }

  reportProgress(50);

  if(!m_data.isEmpty())
  {
    QXmlStreamReader xml(m_data);

    QString newVersion;

    while(!xml.atEnd())
    {
      if (xml.isStartElement() && xml.name() == RELEASE_ELEMENT)
      {
        VersionNumbers otherVersion{xml.attributes().value(VERSION_ELEMENT).toString()};

        if(otherVersion.isValid() && (ESPINAVERSION < otherVersion) && (!SKIP_VERSION.isValid() || SKIP_VERSION < otherVersion))
        {
          if(m_version.isEmpty()) m_version = otherVersion.toString();
          newVersion = otherVersion.toString();
        }
      }

      if(xml.isStartElement() && xml.name() == DETAILS_ELEMENT && !newVersion.isEmpty())
      {
        const auto versionDescription = xml.readElementText(QXmlStreamReader::ErrorOnUnexpectedElement);
        m_versionDescription += tr("<b><u>Version %1</u></b>\n").arg(newVersion) + versionDescription + QString("\n\n");
        newVersion.clear();
      }

      xml.readNextStartElement();
    }

    if(xml.hasError())
    {
      m_error = tr("Error parsing the update data. Error: %1").arg(xml.errorString());
    }
  }
  else
  {
    m_error = tr("Couldn't get the update data. Error: %1").arg(m_error);
  }

  reportProgress(100);
}

//--------------------------------------------------------------------
void UpdateCheck::onDataDownloaded(QNetworkReply *reply)
{
  if(reply->error() == QNetworkReply::NoError)
  {
    m_data = reply->readAll();
    reply->deleteLater();
  }
  else
  {
    m_error = reply->errorString();
  }

  m_loop->quit();
}

//--------------------------------------------------------------------
UpdateCheck::VersionNumbers::VersionNumbers(const QString& versionString)
: VersionNumbers()
{
  bool ok = false;

  if(!versionString.isEmpty() && versionString.count('.') == 2)
  {
    auto parts = versionString.split('.');

    major = parts.first().toInt(&ok);
    if(ok)
    {
      minor = parts.at(1).toInt(&ok);
      if(ok)
      {
        if(parts.last().length() > 2)
        {
          patch = parts.last().split('-').first().toInt(&ok);;
        }
        else
        {
          patch = parts.last().toInt(&ok);
        }
      }
    }
  }

  if(!ok)
  {
    major = minor = patch = 0;
  }
}

//--------------------------------------------------------------------
bool UpdateCheck::VersionNumbers::operator <(const VersionNumbers &other) const
{
  return isValid() && other.isValid() &&
         ((major < other.major) ||
          (major == other.major && minor < other.minor) ||
          (major == other.major && minor == other.minor && patch < other.patch));
}

//--------------------------------------------------------------------
bool UpdateCheck::VersionNumbers::operator ==(const VersionNumbers &other) const
{
  return isValid() && other.isValid() &&
         (major == other.major && minor == other.minor && patch == other.patch);
}

//--------------------------------------------------------------------
bool UpdateCheck::VersionNumbers::operator >(const VersionNumbers &other) const
{
  return isValid() && other.isValid() && !operator==(other) && !operator<(other);
}

//--------------------------------------------------------------------
bool UpdateCheck::VersionNumbers::operator <=(const VersionNumbers &other) const
{
  return isValid() && other.isValid() && (operator<(other) || operator==(other));
}

//--------------------------------------------------------------------
bool UpdateCheck::VersionNumbers::operator >=(const VersionNumbers &other) const
{
  return isValid() && other.isValid() && !operator<(other);
}

bool UpdateCheck::VersionNumbers::operator!=(const VersionNumbers &other) const
{
  return isValid() && other.isValid() && !operator==(other);
}

//--------------------------------------------------------------------
UpdateCheck::VersionNumbers &UpdateCheck::VersionNumbers::operator =(const VersionNumbers &other)
{
  major = other.major;
  minor = other.minor;
  patch = other.patch;

  return *this;
}
