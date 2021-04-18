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

#ifndef CORE_TOOLS_CHANGESEGSPACING_SPACINGCHANGER_H_
#define CORE_TOOLS_CHANGESEGSPACING_SPACINGCHANGER_H_

// Project
#include "ui_SpacingChanger.h"

// ESPINA
#include <Core/Utils/Bounds.h>
#include <Core/Utils/Vector3.hxx>

// Qt
#include <QDialog>
#include <QFileDialog>

/** \class SpacingChanger
 * \brief Application dialog.
 *
 */
class SpacingChanger
: public QDialog
, private Ui_Dialog
{
    Q_OBJECT
  public:
    /** \brief SpacingChanger class constructor.
     * \param[in] parent pointer of the widget parent of this one.
     * \param[in] flags dialog flags.
     *
     */
    explicit SpacingChanger(QWidget *parent = nullptr, Qt::WindowFlags flags = 0);

    /** \brief SpacingChanger class virtual destructor.
     *
     */
    virtual ~SpacingChanger();

  private slots:
    /** \brief Method to process the files.
     *
     */
    void startConversion();

    /** \brief Adds files to conversion process.
     *
     */
    void addFiles();

  private:
    void increaseErrors()
    { m_errors->setText(QString::number(m_errors->text().toInt() + 1)); };

    void increaseConverted()
    { m_converted->setText(QString::number(m_converted->text().toInt() + 1)); };

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

    /** \brief Processes a XML data array.
     * \param[inout] data xml file contents.
     *
     */
    void processXML(QByteArray &data);

    /** \brief Process a Boost graph file array.
     * \param[inout] data Boost graph file contents.
     *
     */
    void processGraph(QByteArray &data);

    /** \brief Process a MHD image header.
     * \param[inout] data MHD header file contents.
     *
     */
    void processMHD(QByteArray &data);

    /** \brief Deserializes the ROI and returns a corrected ROI
     * \param[in] data ROI serialization.
     *
     */
    QByteArray processROI(const QByteArray &data);

    /** \brief Scales the meshes.
     * \param[in] data mesh file contents.
     * \param[in] spacing mesh points spacing.
     *
     */
    QByteArray processMesh(const QByteArray &data, const ESPINA::NmVector3 &spacing);

    /** \brief Scales the stencil files.
     * \param[in] data stencil file contents.
     *
     */
    void processStencil(QByteArray &data);

    /** \brief Purges the extensions data.
     * \param[inout] data xml from an extension data file.
     *
     */
    void purgeInfo(QByteArray &data);

    /** \brief Parses and returns the bounds on the data.
     * \param[in] data bounds data.
     *
     */
    ESPINA::Bounds parseBounds(const QByteArray &data);

    /** \brief Parses and returns the spacing on the data.
     * \param[in] data spacing numbers separated by commas.
     *
     */
    ESPINA::NmVector3 parseSpacing(const QByteArray &data, const QChar separator = ',');

    /** \brief Returns the spacing in the contents.dot file.
     * \param[in] data contents of the contents.dot file.
     *
     */
    ESPINA::NmVector3 getSpacing(const QByteArray &data);

    QList<QFileInfo> m_files;
    ESPINA::NmVector3 m_spacing;
};

#endif // CORE_TOOLS_CHANGESEGSPACING_SPACINGCHANGER_H_
