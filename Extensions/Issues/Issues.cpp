/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2015  Jorge Peña Pastor <jpena@cesvima.upm.es>
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

#include "Issues.h"

using namespace ESPINA::Extensions;

QString Issue::INFORMATION_TAG = "information";
QString Issue::WARNING_TAG     = "warning";
QString Issue::CRITICAL_TAG    = "critical";

//------------------------------------------------------------------------
QString Issue::displayName() const
{
  return m_itemName;
}

//------------------------------------------------------------------------
Issue::Severity Issue::severity() const
{
  return m_severity;
}

//------------------------------------------------------------------------
QString Issue::description() const
{
  return m_description;
}

//------------------------------------------------------------------------
QString Issue::suggestion() const
{
  return m_suggestion;
}
