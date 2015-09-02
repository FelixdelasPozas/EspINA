/*
 *
 * Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 * This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

// ESPINA
#include "DefaultDialogs.h"
#include <GUI/Widgets/Styles.h>

// Qt
#include <QMessageBox>
#include <QDebug>

using namespace ESPINA;
using namespace ESPINA::GUI;
using namespace ESPINA::GUI::Widgets::Styles;

//------------------------------------------------------------------------
QString DefaultDialogs::DefaultPath()
{
  return QString();
}

//------------------------------------------------------------------------
QString DefaultDialogs::OpenFile(const QString       &title,
                                 const SupportedFormats &filters,
                                 const QString       &path)
{
  QString fileName;

  auto fileNames = OpenFiles(title, filters, path);
  if (!fileNames.isEmpty())
  {
    fileName = fileNames.first();
  }

  return fileName;
}

//------------------------------------------------------------------------
QStringList DefaultDialogs::OpenFiles(const QString       &title,
                                      const SupportedFormats &filters,
                                      const QString       &path,
                                      const QList<QUrl>   &recent)
{
  QStringList fileNames;

  QFileDialog fileDialog;
  fileDialog.setWindowTitle(title);
  fileDialog.setWindowFlags(Qt::WindowStaysOnTopHint);
  fileDialog.setDirectory(path);
  fileDialog.setNameFilters(filters);
  fileDialog.setFileMode(QFileDialog::ExistingFiles);
  fileDialog.setViewMode(QFileDialog::Detail);
  fileDialog.setOption(QFileDialog::DontUseNativeDialog, true);
  fileDialog.resize(800, 480);
  fileDialog.setAcceptMode(QFileDialog::AcceptOpen);

  QList<QUrl> urls;

  urls << recent << fileDialog.sidebarUrls();

  fileDialog.setSidebarUrls(urls);

  DefaultCursor cursor;
  
  if (fileDialog.exec() == QDialog::Accepted)
  {
    fileNames = fileDialog.selectedFiles();
  }

  return fileNames;
}

//------------------------------------------------------------------------
QDir DefaultDialogs::SaveDirectory(const QString& title, const QString& path)
{
  QDir dir;

  QFileDialog fileDialog;
  fileDialog.setWindowTitle(title);
  fileDialog.setWindowFlags(Qt::WindowStaysOnTopHint);
  fileDialog.setFileMode(QFileDialog::DirectoryOnly);
  fileDialog.setDirectory(path);
  fileDialog.setViewMode(QFileDialog::Detail);
  fileDialog.setOption(QFileDialog::DontUseNativeDialog, true);
  fileDialog.resize(800, 480);
  fileDialog.setAcceptMode(QFileDialog::AcceptSave);

  DefaultCursor cursor;

  if (fileDialog.exec() == QDialog::Accepted)
  {
    dir = fileDialog.directory();
  }

  return dir;
}


//------------------------------------------------------------------------
QString DefaultDialogs::SaveFile(const QString& title,
                                 const SupportedFormats& filters,
                                 const QString& path,
                                 const QString& suffix,
                                 const QString& suggestion)
{
  QString fileName;

  auto fileNames = SaveFiles(title, filters, path, suffix, suggestion);
  if (!fileNames.isEmpty())
  {
    fileName = fileNames.first();
  }

  return fileName;
}

//------------------------------------------------------------------------
QStringList DefaultDialogs::SaveFiles(const QString& title,
                                      const SupportedFormats& filters,
                                      const QString& path,
                                      const QString& suffix,
                                      const QString& suggestion)
{
  QStringList fileNames;

  QFileDialog fileDialog;
  fileDialog.setWindowTitle(title);
  fileDialog.setWindowFlags(Qt::WindowStaysOnTopHint);
  fileDialog.setDefaultSuffix(suffix);
  fileDialog.setFileMode(QFileDialog::AnyFile);
  fileDialog.selectFile(suggestion);
  fileDialog.setFilter(filters);
  fileDialog.setDirectory(path);
  fileDialog.setViewMode(QFileDialog::Detail);
  fileDialog.setOption(QFileDialog::DontUseNativeDialog, true);
  fileDialog.resize(800, 480);
  fileDialog.setConfirmOverwrite(true);
  fileDialog.setAcceptMode(QFileDialog::AcceptSave);

  DefaultCursor cursor;

  if (fileDialog.exec() == QDialog::Accepted)
  {
    fileNames = fileDialog.selectedFiles();

    QRegExp regExp("\\*\\.[a-zA-Z0-9]+"); // file extension *.XXX
    regExp.indexIn(fileDialog.selectedFilter());

    auto extension = regExp.cap().remove("*");

    for (auto &fileName : fileNames)
    {
      if (!fileName.endsWith(extension))
      {
        fileName += extension;
      }
    }
  }

  return fileNames;
}

//------------------------------------------------------------------------
QString DefaultDialogs::DefaultTitle()
{
  return QObject::tr("ESPINA");
}

//------------------------------------------------------------------------
bool DefaultDialogs::UserConfirmation(const QString& message, const QString& title)
{
  QMessageBox dialog;

  dialog.setWindowTitle(title);
  dialog.setText(message);
  dialog.setStandardButtons(QMessageBox::Ok|QMessageBox::Cancel);

  DefaultCursor cursor;

  return dialog.exec() == QMessageBox::Ok;
}

//------------------------------------------------------------------------
void DefaultDialogs::InformationMessage(const QString& message, const QString& title)
{
  QMessageBox dialog;

  dialog.setWindowTitle(title);
  dialog.setText(message);
  dialog.setStandardButtons(QMessageBox::Ok);
  dialog.setModal(true);

  DefaultCursor cursor;

  dialog.exec();
}
