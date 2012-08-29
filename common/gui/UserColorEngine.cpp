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

#include "UserColorEngine.h"

#include <common/model/Segmentation.h>

//-----------------------------------------------------------------------------
UserColorEngine::UserColorEngine() :
m_lastColor(0)
{
  m_colors << QColor(31, 120, 180) << QColor(51, 160, 44) << QColor(227, 26, 28) << QColor(255, 127, 0) << QColor(106, 61, 154) << QColor(166, 206, 227)
  << QColor(178, 223, 138) << QColor(251, 154, 153) << QColor(253, 191, 111) << QColor(202, 178, 214);
}

//-----------------------------------------------------------------------------
vtkSmartPointer< vtkLookupTable > UserColorEngine::lut(const Segmentation* seg)
{
  return NULL;
}

//-----------------------------------------------------------------------------
QColor UserColorEngine::color(const Segmentation* seg)
{
  QString user = seg->users().last();

  if (!m_userColors.contains(user))
  {
    m_userColors[user] = nextColor();
  }

  return m_userColors[user];
}

//-----------------------------------------------------------------------------
QColor UserColorEngine::nextColor()
{
  return m_colors[m_lastColor++];
}

