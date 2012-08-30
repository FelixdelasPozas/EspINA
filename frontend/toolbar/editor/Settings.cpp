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


#include "Settings.h"

#include <QSettings>

const QString BRUSH ("EditorToolBar::BrushRadius" );
const QString ERODE ("EditorToolBar::ErodeRadius" );
const QString DILATE("EditorToolBar::DilateRadius");
const QString OPEN  ("EditorToolBar::OpenRadius"  );
const QString CLOSE ("EditorToolBar::CloseRadius" );

//------------------------------------------------------------------------
EditorToolBar::Settings::Settings()
{
  QSettings settings("CeSViMa", "EspINA");

  m_brushRadius  = settings.value(BRUSH, 20).toInt();
  m_erodeRadius  = settings.value(ERODE, 3).toInt();
  m_dilateRadius = settings.value(DILATE, 3).toInt();
  m_openRadius   = settings.value(OPEN, 3).toInt();
  m_closeRadius  = settings.value(CLOSE, 3).toInt();
}

//------------------------------------------------------------------------
void EditorToolBar::Settings::setBrushRadius(int r)
{
  QSettings settings("CeSViMa", "EspINA");

  settings.setValue(BRUSH, r);
  m_brushRadius = r;
}

//------------------------------------------------------------------------
void EditorToolBar::Settings::setErodeRadius(int r)
{
  QSettings settings("CeSViMa", "EspINA");

  settings.setValue(ERODE, r);
  m_erodeRadius = r;
}

//------------------------------------------------------------------------
void EditorToolBar::Settings::setDilateRadius(int r)
{
  QSettings settings("CeSViMa", "EspINA");

  settings.setValue(DILATE, r);
  m_dilateRadius = r;
}

//------------------------------------------------------------------------
void EditorToolBar::Settings::setOpenRadius(int r)
{
  QSettings settings("CeSViMa", "EspINA");

  settings.setValue(OPEN, r);
  m_openRadius = r;
}

//------------------------------------------------------------------------
void EditorToolBar::Settings::setCloseRadius(int r)
{
  QSettings settings("CeSViMa", "EspINA");

  settings.setValue(CLOSE, r);
  m_closeRadius = r;
}