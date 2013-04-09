/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  <copyright holder> <email>

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


#ifndef EDITORTOOLBAR_SETTINGS_H
#define EDITORTOOLBAR_SETTINGS_H

#include <App/Toolbars/Editor/EditorToolBar.h>

class EspINA::EditorToolBar::Settings
{
public:
  Settings();

  void setBrushRadius(int r);
  int brushRadius() const
  {return m_brushRadius;}

  void setErodeRadius(int r);
  int erodeRadius() const
  {return m_erodeRadius;}

  void setDilateRadius(int r);
  int dilateRadius() const
  {return m_dilateRadius;}

  void setOpenRadius(int r);
  int openRadius() const
  {return m_openRadius;}

  void setCloseRadius(int r);
  int closeRadius() const
  {return m_closeRadius;}


private:
  int m_brushRadius;
  int m_erodeRadius;
  int m_dilateRadius;
  int m_openRadius;
  int m_closeRadius;
};

#endif // EDITORTOOLBAR_SETTINGS_H
