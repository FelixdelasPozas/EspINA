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

// Qt
#include <QMessageBox>
#include <QDebug>

using namespace ESPINA;
using namespace ESPINA::GUI;

//------------------------------------------------------------------------
SupportedFiles::SupportedFiles()
{

}

//------------------------------------------------------------------------
SupportedFiles::SupportedFiles(const QString &name, const QString &extension)
{
  addFilter(name, extension);
}

//------------------------------------------------------------------------
SupportedFiles &SupportedFiles::addFormat(const QString &name, const QString &extension)
{
  addFilter(name, extension);

  return *this;
}

//------------------------------------------------------------------------
SupportedFiles::operator QString() const
{
  return m_filter;
}

//------------------------------------------------------------------------
void SupportedFiles::addFilter(const QString &name, const QString &extension)
{
  if (!m_filter.isEmpty())
  {
    m_filter += ";;";
  }

  m_filter += QString("%1 (.%2)").arg(name).arg(extension);
}

//------------------------------------------------------------------------
QString DefaultDialogs::OpenFile(const QString& title, const QStringList& filters, const QString& path)
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
QStringList DefaultDialogs::OpenFiles(const QString& title, const QStringList& filters, const QString& path)
{
  QStringList fileNames;

  QFileDialog fileDialog;
  fileDialog.setWindowTitle(title);
  fileDialog.setDirectory(path);
  fileDialog.setNameFilters(filters);
  fileDialog.setFileMode(QFileDialog::ExistingFiles);
  fileDialog.setViewMode(QFileDialog::Detail);
  fileDialog.setOption(QFileDialog::DontUseNativeDialog, false);
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
                                 const SupportedFiles& filters,
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
                                      const SupportedFiles& filters,
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
  fileDialog.setViewMode(QFileDialog::Detail);
  fileDialog.resize(800, 480);
  fileDialog.setConfirmOverwrite(true);
  fileDialog.setAcceptMode(QFileDialog::AcceptSave);

  if (fileDialog.exec() == QDialog::Accepted)
  {
    fileNames = fileDialog.selectedFiles();

    auto extension = fileDialog.selectedFilter();
    extension      = extension.mid(extension.indexOf("(.")+1,4);

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
bool DefaultDialogs::UserConfirmation(const QString& title, const QString& message)
{
  QMessageBox dialog;

  dialog.setWindowTitle(title);
  dialog.setText(message);
  dialog.setStandardButtons(QMessageBox::Ok|QMessageBox::Cancel);

  return dialog.exec() == QMessageBox::Ok;
}

//------------------------------------------------------------------------
void DefaultDialogs::InformationMessage(const QString& title, const QString& message)
{
  QMessageBox dialog;

  dialog.setWindowTitle(title);
  dialog.setText(message);
  dialog.setStandardButtons(QMessageBox::Ok);

  dialog.exec();
}
