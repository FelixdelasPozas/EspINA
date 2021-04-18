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

// ESPINA
#include <Support/Settings/Settings.h>
#include "RecentDocuments.h"
#include <QAction>
#include <QSettings>
#include <QStringList>
#include <QFileInfo>
#include <QUrl>

const int MAX_FILES = 10;
const QString RECENT_DOCUMENTS = "RecentDocuments";

//------------------------------------------------------------------------
RecentDocuments::RecentDocuments()
{
  loadRecentDocuments();
}

//------------------------------------------------------------------------
RecentDocuments::~RecentDocuments()
{
}

//------------------------------------------------------------------------
void RecentDocuments::addDocument(QString path)
{
  m_recentDocuments.removeOne(path);

  if (m_recentDocuments.size() == MAX_FILES)
  {
    m_recentDocuments.pop_back();
  }

  m_recentDocuments.push_front(path);

  saveRecentDocuments();
}

//------------------------------------------------------------------------
void RecentDocuments::removeDocument(QString path)
{
  m_recentDocuments.removeOne(path);

  saveRecentDocuments();
}

#include <QDebug>
//------------------------------------------------------------------------
QList<QUrl> RecentDocuments::recentDocumentUrls() const
{
  QList<QUrl> urls;

  for (QFileInfo document : m_recentDocuments)
  {
    urls << QUrl::fromLocalFile(document.absolutePath());
  }

  return urls;
}

//------------------------------------------------------------------------
void RecentDocuments::loadRecentDocuments()
{
  ESPINA_SETTINGS(settings);

  m_recentDocuments = settings.value(RECENT_DOCUMENTS, QStringList()).toStringList();
}

//------------------------------------------------------------------------
void RecentDocuments::saveRecentDocuments()
{
  ESPINA_SETTINGS(settings);

  settings.setValue(RECENT_DOCUMENTS, m_recentDocuments);
  settings.sync();
}
