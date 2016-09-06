/*
 *    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 *    This file is part of ESPINA.
 *
 *    ESPINA is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

// ESPINA
#include <Core/Utils/SupportedFormats.h>

using namespace ESPINA::Core::Utils;

//------------------------------------------------------------------------
SupportedFormats::SupportedFormats()
{
}

//------------------------------------------------------------------------
SupportedFormats::SupportedFormats(const QString &name, const QString &extension)
{
  addFormat(name, extension);
}

//------------------------------------------------------------------------
SupportedFormats::SupportedFormats(const QString &name, const QStringList &extensions)
{
  addFormat(name, extensions);
}


//------------------------------------------------------------------------
SupportedFormats &SupportedFormats::addFormat(const QString &name, const QString &extension)
{
  addFilter(name, extension);

  return *this;
}

//------------------------------------------------------------------------
SupportedFormats &SupportedFormats::addFormat(const QString &name, const QStringList &extensions)
{
  addFilter(name, extensions.join(" *."));
  return *this;
}

//------------------------------------------------------------------------
SupportedFormats &SupportedFormats::addAllFormat()
{
  addFormat(QObject::tr("All"), "*");

  return *this;
}

//------------------------------------------------------------------------
SupportedFormats &SupportedFormats::addCSVFormat()
{
  addFormat(QObject::tr("CSV Text File"), "csv");

  return *this;
}

//------------------------------------------------------------------------
SupportedFormats &SupportedFormats::addExcelFormat()
{
  addFormat(QObject::tr("Excel Sheet"), "xls");

  return *this;
}

//------------------------------------------------------------------------
SupportedFormats &SupportedFormats::addSegFormat()
{
  addFormat(QObject::tr("EspINA Analysis"), "seg");

  return *this;
}

//------------------------------------------------------------------------
SupportedFormats &SupportedFormats::addTxtFormat()
{
  addFormat(QObject::tr("Text File"), "txt");

  return *this;
}

//------------------------------------------------------------------------
SupportedFormats::operator QString() const
{
  return m_filters.join(";;");
}

//------------------------------------------------------------------------
SupportedFormats::operator QStringList() const
{
  return m_filters;
}

//------------------------------------------------------------------------
bool SupportedFormats::operator==(const SupportedFormats &rhs) const
{
  return m_filters == rhs.m_filters;
}

//------------------------------------------------------------------------
void SupportedFormats::addFilter(const QString &name, const QString &extension)
{
  m_filters << QString("%1 (*.%2)").arg(name).arg(extension);
}
