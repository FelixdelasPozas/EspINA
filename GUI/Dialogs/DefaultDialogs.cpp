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
#include <QApplication>
#include <QMainWindow>

using namespace ESPINA;
using namespace ESPINA::GUI;
using namespace ESPINA::GUI::Widgets::Styles;

//------------------------------------------------------------------------
QWidget *DefaultDialogs::defaultParentWidget()
{
  auto widgets = QApplication::topLevelWidgets();

  for(auto widget: widgets)
  {
    if(widget->inherits("QMainWindow"))
    {
      auto mainWin = qobject_cast<QMainWindow *>(widget);

      return mainWin->centralWidget();
    }
  }

  return nullptr;
}

//------------------------------------------------------------------------
QString DefaultDialogs::DefaultPath()
{
  return QString();
}

//------------------------------------------------------------------------
QString DefaultDialogs::OpenFile(const QString          &title,
                                 const SupportedFormats &filters,
                                 const QString          &path,
                                 const QList<QUrl>      &recent,
                                 QWidget                *parent)
{
  QString fileName;

  auto fileNames = OpenFiles(title, filters, path, recent,  parent);
  if (!fileNames.isEmpty())
  {
    fileName = fileNames.first();
  }

  return fileName;
}

//------------------------------------------------------------------------
QStringList DefaultDialogs::OpenFiles(const QString          &title,
                                      const SupportedFormats &filters,
                                      const QString          &path,
                                      const QList<QUrl>      &recent,
                                      QWidget                *parent)
{
  QStringList fileNames;

  QFileDialog fileDialog(parent);
  fileDialog.setWindowTitle(title);
  fileDialog.setDirectory(path);
  fileDialog.setNameFilters(filters);
  fileDialog.setFileMode(QFileDialog::ExistingFiles);
  fileDialog.setViewMode(QFileDialog::Detail);
  fileDialog.setOption(QFileDialog::DontUseNativeDialog, true);
  fileDialog.resize(800, 480);
  fileDialog.setAcceptMode(QFileDialog::AcceptOpen);
  fileDialog.setModal(true);

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
QDir DefaultDialogs::SaveDirectory(const QString& title, const QString& path, QWidget *parent)
{
  QDir dir;

  QFileDialog fileDialog(parent);
  fileDialog.setWindowTitle(title);
  fileDialog.setFileMode(QFileDialog::DirectoryOnly);
  fileDialog.setDirectory(path);
  fileDialog.setViewMode(QFileDialog::Detail);
  fileDialog.setOption(QFileDialog::DontUseNativeDialog, true);
  fileDialog.resize(800, 480);
  fileDialog.setAcceptMode(QFileDialog::AcceptSave);
  fileDialog.setModal(true);

  DefaultCursor cursor;

  if (fileDialog.exec() == QDialog::Accepted)
  {
    dir = fileDialog.directory();
  }

  return dir;
}


//------------------------------------------------------------------------
QString DefaultDialogs::SaveFile(const QString&          title,
                                 const SupportedFormats& filters,
                                 const QString&          path,
                                 const QString&          suffix,
                                 const QString&          suggestion,
                                 QWidget                *parent)
{
  QString fileName;

  auto fileNames = SaveFiles(title, filters, path, suffix, suggestion, parent);
  if (!fileNames.isEmpty())
  {
    fileName = fileNames.first();
  }

  return fileName;
}

//------------------------------------------------------------------------
QStringList DefaultDialogs::SaveFiles(const QString&          title,
                                      const SupportedFormats& filters,
                                      const QString&          path,
                                      const QString&          suffix,
                                      const QString&          suggestion,
                                      QWidget                *parent)
{
  QStringList fileNames;

  QFileDialog fileDialog(parent);
  fileDialog.setWindowTitle(title);
  fileDialog.setDefaultSuffix(suffix);
  fileDialog.setFileMode(QFileDialog::AnyFile);
  fileDialog.selectFile(suggestion);
  fileDialog.setFilter(filters);
  fileDialog.setDirectory((path.isEmpty() ? QDir::homePath() : path));
  fileDialog.setViewMode(QFileDialog::Detail);
  fileDialog.setOption(QFileDialog::DontUseNativeDialog, true);
  fileDialog.resize(800, 480);
  fileDialog.setConfirmOverwrite(true);
  fileDialog.setAcceptMode(QFileDialog::AcceptSave);
  fileDialog.setModal(true);

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
void DefaultDialogs::InformationMessage(const QString& message, const QString& title, const QString &details, QWidget *parent)
{
  QMessageBox dialog(parent);

  dialog.setWindowTitle(title);
  dialog.setText(message);
  dialog.setStandardButtons(QMessageBox::Ok);
  if(!details.isEmpty())
  {
    dialog.setDetailedText(details);
  }
  dialog.setModal(true);
  dialog.setIcon(QMessageBox::Information);

  DefaultCursor cursor;

  dialog.exec();
}

//------------------------------------------------------------------------
QMessageBox::StandardButton DefaultDialogs::UserQuestion(const QString                     &message,
                                                         const QMessageBox::StandardButtons buttons,
                                                         const QString                     &title,
                                                         const QString                     &details,
                                                         QWidget                           *parent)
{
  QMessageBox dialog(parent);

  dialog.setWindowTitle(title);
  dialog.setText(message);

  if(!details.isEmpty())
  {
    dialog.setDetailedText(details);
  }

  dialog.setStandardButtons(buttons);

  if(buttons.testFlag(QMessageBox::Discard)) dialog.setButtonText(QMessageBox::Discard, QObject::tr("Discard"));

  dialog.setModal(true);
  dialog.setIcon(QMessageBox::Question);

  DefaultCursor cursor;

  return static_cast<QMessageBox::StandardButton>(dialog.exec());
}

//------------------------------------------------------------------------
void DefaultDialogs::ErrorMessage(const QString& message, const QString& title, const QString &details, QWidget *parent)
{
  QMessageBox dialog(parent);

  dialog.setWindowTitle(title);
  dialog.setText(message);
  dialog.setStandardButtons(QMessageBox::Ok);
  if(!details.isEmpty())
  {
    dialog.setDetailedText(details);
  }
  dialog.setModal(true);
  dialog.setIcon(QMessageBox::Warning);

  DefaultCursor cursor;

  dialog.exec();
}

