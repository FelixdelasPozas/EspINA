/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  Jorge Peña Pastor <jpena@cesvima.upm.es>

    This program is free software: you can redistribute it and/or modify
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


#include "RecentDocuments.h"
#include <Core/EspinaSettings.h>

#include <QAction>
#include <QSettings>
#include <QStringList>
#include <QFileInfo>

const int MAX_FILES = 10;
//------------------------------------------------------------------------
RecentDocuments::RecentDocuments()
{
  updateDocumentList();
}

//------------------------------------------------------------------------
RecentDocuments::~RecentDocuments()
{
  QSettings settings(CESVIMA, ESPINA);
  settings.setValue("recentFileList", m_recentDocuments);
  settings.sync();
}

//------------------------------------------------------------------------
void RecentDocuments::addDocument(QString path)
{
  if (m_recentDocuments.contains(path))
    m_recentDocuments.removeAll(path);

  if (m_recentDocuments.size() == MAX_FILES)
    m_recentDocuments.pop_back();

  m_recentDocuments.push_front(path);

  updateActions();

  QSettings settings(CESVIMA, ESPINA);
  settings.setValue("recentFileList", m_recentDocuments);
  settings.sync();
}

//------------------------------------------------------------------------
void RecentDocuments::removeDocument(QString path)
{
  if (m_recentDocuments.contains(path))
    m_recentDocuments.removeAll(path);

  updateActions();

  QSettings settings(CESVIMA, ESPINA);
  settings.setValue("recentFileList", m_recentDocuments);
  settings.sync();
}


//------------------------------------------------------------------------
void RecentDocuments::updateActions()
{
  int numberFiles = qMin(m_recentDocuments.size(), MAX_FILES);
  for(int i = 0; i < numberFiles; i++)
  {
    m_actionList[i]->setText(QFileInfo(m_recentDocuments[i]).fileName());
    m_actionList[i]->setToolTip(m_recentDocuments[i]);
    m_actionList[i]->setData(m_recentDocuments[i]);
    m_actionList[i]->setVisible(true);
  }

  for(int i = numberFiles; i < MAX_FILES; i++)
    m_actionList[i]->setVisible(false);
}

//------------------------------------------------------------------------
void RecentDocuments::updateDocumentList()
{
  QSettings settings(CESVIMA, ESPINA);

  if (settings.contains("recentFileList"))
    m_recentDocuments = settings.value("recentFileList").toStringList();
  else
    m_recentDocuments = QStringList();

  for(int i = 0; i < MAX_FILES; i++)
  {
    QAction *action = new QAction(this);
    action->setVisible(false);
    m_actionList << action;
  }

  updateActions();
}
