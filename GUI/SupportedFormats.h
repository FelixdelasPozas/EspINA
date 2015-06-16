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

#ifndef ESPINA_GUI_SUPPORTED_FORMATS_H
#define ESPINA_GUI_SUPPORTED_FORMATS_H

#include <QStringList>

namespace ESPINA
{
  namespace GUI
  {
    class SupportedFormats
    {
    public:
      SupportedFormats();

      SupportedFormats(const QString &name, const QString &extension);

      SupportedFormats(const QString &name, const QStringList &extensions);

      SupportedFormats &addFormat(const QString &name, const QString &extension);

      SupportedFormats &addFormat(const QString &name, const QStringList &extensions);

      operator QString() const;

      operator QStringList() const;

      bool operator==(const SupportedFormats &rhs) const;

      SupportedFormats& addAllFormat();

      SupportedFormats& addCSVFormat();

      SupportedFormats& addExcelFormat();

      SupportedFormats& addSegFormat();

      SupportedFormats& addTxtFormat();

    private:
      void addFilter(const QString &name, const QString &extension);

    private:
      QStringList m_filters;
    };
  }
}

#endif // ESPINA_GUI_SUPPORTED_FORMATS_H
