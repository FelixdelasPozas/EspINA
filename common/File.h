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


#ifndef FILE_H
#define FILE_H

#include <QString>

class File : public QString
{
public:
  explicit File(const QString file);

  static QString extension(const QString file);
  QString extension() const;
  static QString extendedName(const QString file);
  QString extendedName() const;
  static QString name(const QString file);
  QString name() const;
  static QString parentDirectory(const QString file);
  QString parentDirectory() const;

private:
//   const QString m_file;
};

#endif // FILE_H
