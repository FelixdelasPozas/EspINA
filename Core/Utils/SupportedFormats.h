/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2015  <copyright holder> <email>
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

#ifndef ESPINA_CORE_SUPPORTED_FORMATS_H
#define ESPINA_CORE_SUPPORTED_FORMATS_H

#include "Core/EspinaCore_Export.h"

// Qt
#include <QStringList>

namespace ESPINA
{
  namespace Core
  {
    namespace Utils
    {
      /** \class SupportedFormats
       * \brief Helper class to define various filters for EspINA session generated files.
       */
      class EspinaCore_EXPORT SupportedFormats
      {
        public:
          /** \brief SupportedFormats class constructor.
           *
           */
          SupportedFormats();

          /** \brief SupportedFormats class constructor.
           * \param[in] name file filter name.
           * \param[in] extension file extension.
           *
           */
          SupportedFormats(const QString &name, const QString &extension);

          /** \brief SupportedFormats class constructor.
           * \param[in] name file filter name.
           * \param[in] extensions file extensions.
           *
           */
          SupportedFormats(const QString &name, const QStringList &extensions);

          /** \brief Adds a new format to the list of formats.
           * \param[in] name file filter name.
           * \param[in] extension file extension.
           *
           */
          SupportedFormats &addFormat(const QString &name, const QString &extension);

          /** \brief Adss a new format to the list of formats (multiple extensions).
           * \param[in] name file filter name.
           * \param[in] extensions file extensions.
           *
           */
          SupportedFormats &addFormat(const QString &name, const QStringList &extensions);

          /** \brief operator QString()
           *
           */
          operator QString() const;

          /** \brief operator QStringList()
           *
           */
          operator QStringList() const;

          /** \brief operator ==
           *
           */
          bool operator==(const SupportedFormats &rhs) const;

          /** \brief Adds "all" format to the format list.
           *
           */
          SupportedFormats& addAllFormat();

          /** \brief Adds "csv" format to the format list.
           *
           */
          SupportedFormats& addCSVFormat();

          /** \brief Adds "xls" format to the format list.
           *
           */
          SupportedFormats& addExcelFormat();

          /** \brief Adds "seg" format to the format list.
           *
           */
          SupportedFormats& addSegFormat();

          /** \brief Adds "txt" format to the format list.
           *
           */
          SupportedFormats& addTxtFormat();

        private:
          /** \brief Helper method to add a filter and file extensions to the format list.
           *
           */
          void addFilter(const QString &name, const QString &extension);

        private:
          QStringList m_filters; /** format list as a list of strings in QFileDialog filters format. */
      };
    } // namespace Utils
  } // namespace Core
} // namespace ESPINA

#endif // ESPINA_CORE_SUPPORTED_FORMATS_H
