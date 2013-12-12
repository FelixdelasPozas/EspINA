/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2013  Jorge Peña Pastor <jpena@cesvima.upm.es>
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

#include "EspinaErrorHandler.h"

#include <QUrl>
#include <QDesktopServices>
#include <QFileDialog>
#include <QApplication>
#include <QMessageBox>

using namespace EspINA;

QFileInfo EspinaErrorHandler::fileNotFound(const QFileInfo& file, QDir dir, const QString& nameFilters, const QString& hint)
{
  QString key = file.absoluteFilePath();
  if (!m_files.contains(key))
  {
    QString title     = (hint.isEmpty())        ? QObject::tr("Select file for %1:").arg(file.fileName()) : hint;
    QDir    directory = (dir == QDir())         ? m_defaultDir : dir;
    QString filters   = (nameFilters.isEmpty()) ? QObject::tr("%1 files (*.%1)").arg(file.suffix()) : nameFilters;

    QList<QUrl> urls;
    urls << QUrl::fromLocalFile(QDesktopServices::storageLocation(QDesktopServices::DesktopLocation))
    << QUrl::fromLocalFile(QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation))
    << QUrl::fromLocalFile(QDesktopServices::storageLocation(QDesktopServices::HomeLocation))
    << QUrl::fromLocalFile(QDesktopServices::storageLocation(QDesktopServices::DataLocation));

    QFileDialog fileDialog(m_parent);
    fileDialog.setFileMode(QFileDialog::ExistingFiles);
    fileDialog.setWindowTitle(title);
    fileDialog.setFilter(filters);
    fileDialog.setDirectory(directory);
    fileDialog.setOption(QFileDialog::DontUseNativeDialog, false);
    fileDialog.setViewMode(QFileDialog::Detail);
    fileDialog.resize(800, 480);
    fileDialog.setSidebarUrls(urls);

    QApplication::setOverrideCursor(Qt::ArrowCursor);
    if (fileDialog.exec())
    {
      m_files[key] = QFileInfo(fileDialog.selectedFiles().first());
    }
    QApplication::restoreOverrideCursor();
  }

  return m_files[key];
}

void EspinaErrorHandler::error(const QString& msg)
{
  QMessageBox::warning(m_parent, "EspINA", msg);
}

void EspinaErrorHandler::warning(const QString& msg)
{
  QMessageBox::warning(m_parent, "EspINA", msg);
}
