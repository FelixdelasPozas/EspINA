/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 * This program is free software: you can redistribute it and/or modify
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

#include "DefaultDialogs.h"
#include <QMessageBox>

using namespace EspINA;
using namespace EspINA::GUI;

//------------------------------------------------------------------------
QString DefaultDialogs::OpenFile(const QString& title, const QString& filters, const QString& path)
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
QStringList DefaultDialogs::OpenFiles(const QString& title, const QString& filters, const QString& path)
{
  QStringList fileNames;

  QFileDialog fileDialog;
  fileDialog.setWindowTitle(title);
  fileDialog.setDirectory(path);
  fileDialog.setFileMode(QFileDialog::ExistingFiles);
  fileDialog.setFilter(filters);
  fileDialog.setViewMode(QFileDialog::Detail);
  fileDialog.resize(800, 480);
  fileDialog.setAcceptMode(QFileDialog::AcceptOpen);

  if (fileDialog.exec() == QDialog::Accepted)
  {
    fileNames = fileDialog.selectedFiles();
  }

  return fileNames;
}

//------------------------------------------------------------------------
QString DefaultDialogs::SaveFile(const QString& title,
                                 const QString& filters,
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
                                      const QString& filters,
                                      const QString& path,
                                      const QString& suffix,
                                      const QString& suggestion)
{
  QStringList fileNames;

  QFileDialog fileDialog;
  fileDialog.setWindowTitle(title);
  fileDialog.setDefaultSuffix(suffix);
  fileDialog.setFileMode(QFileDialog::AnyFile);
  fileDialog.selectFile(suggestion);
  fileDialog.setFilter(filters);
  fileDialog.setDirectory(path);
  //fileDialog.setOption(QFileDialog::DontUseNativeDialog, false);
  //fileDialog.setSidebarUrls(urls);
  fileDialog.setViewMode(QFileDialog::Detail);
  fileDialog.resize(800, 480);
  fileDialog.setConfirmOverwrite(true);
  fileDialog.setAcceptMode(QFileDialog::AcceptSave);

  if (fileDialog.exec() == QDialog::Accepted)
  {
    fileNames = fileDialog.selectedFiles();
  }

  return fileNames;
}

//------------------------------------------------------------------------
QMessageBox::StandardButton DefaultDialogs::ConfirmationDialog(const QString& title, const QString& message)
{
  QMessageBox dialog;

  dialog.setWindowTitle(title);
  dialog.setText(message);
  dialog.setStandardButtons(QMessageBox::Ok|QMessageBox::Cancel);

  return dialog.exec();
}
