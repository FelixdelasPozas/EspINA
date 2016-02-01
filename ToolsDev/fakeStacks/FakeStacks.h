/*

 Copyright (C) 2016 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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

#ifndef TOOLSDEV_FAKESTACKS_FAKESTACKS_H_
#define TOOLSDEV_FAKESTACKS_FAKESTACKS_H_

// Project
#include "ui_FakeStacks.h"

// ESPINA
#include <Core/Utils/Vector3.hxx>
#include <Core/Utils/Bounds.h>

//Qt
#include <QDialog>
#include <QDir>

// Quazip
#include <quazip/quazip.h>

class FakeStacks
: public QDialog
, private Ui::FakeStacks
{
    Q_OBJECT
  public:
    /** \brief FakeStacks class constructor.
     * \param[in] parent raw pointer of the widget parent of this one.
     * \param[in] flags window flags.
     *
     */
    explicit FakeStacks(QWidget *parent = nullptr, Qt::WindowFlags flags = 0);

    /** \brief FakeStacks class virtual destructor.
     *
     */
    virtual ~FakeStacks();

  private slots:
    void addFiles();
    void startGeneration();

  private:
    void parseStacks(const QByteArray &data);
    void parseBounds(QuaZip &zip);

    void writeInfo(const QString &information)
    { m_log->setTextColor(Qt::black); m_log->append(information); }

    void writeError(const QString &error)
    { m_log->setTextColor(Qt::red); m_log->append(error);  }

    void writeImportant(const QString &important)
    { m_log->setTextColor(Qt::blue); m_log->append(important); }

    /** \brief Returns true if the file is a SEG version 6 and false otherwise.
     * \param[in] fileInfo formatInfo.ini contents.
     *
     */
    bool isVersion6file(const QByteArray &fileInfo);

    /** \brief Parses and returns the spacing on the data.
     * \param[in] data spacing numbers separated by commas.
     *
     */
    ESPINA::NmVector3 parseSpacing(const QByteArray &data, const QChar separator = ',');

    /** \brief Parses and returns the bounds on the data.
     * \param[in] data bounds data.
     *
     */
    ESPINA::Bounds parseBounds(const QByteArray &data);

    void generateStacks(const QString &path);

    QList<QFileInfo>                 m_files;   /** SEG files to generate stacks.  */
    QMap<QString, ESPINA::Bounds>    m_stacks;  /** stacks bounds in the SEG file. */
    QMap<QString, QString>           m_ids;     /** stacks ids in the SEG file.    */
    QMap<QString, ESPINA::NmVector3> m_spacing; /** stack spacing in SEG file.     */
    QMap<QString, QString>           m_dirs;    /** dirs the of the SEG files.     */
};

#endif // TOOLSDEV_FAKESTACKS_FAKESTACKS_H_
