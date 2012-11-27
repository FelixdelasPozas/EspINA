/*
 <one line to give the program's name and a brief idea of what it does.>
 Copyright (C) 2011  Jorge Peña <jorge.pena.pastor@gmail.com>

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

#include "SettingsPanel.h"

#include <Toolbars/Editor/Settings.h>

#include <QSettings>
#include <QString>

//------------------------------------------------------------------------
EditorToolBar::SettingsPanel::SettingsPanel(EditorToolBar::Settings *settings)
: m_settings(settings)
{
  setupUi(this);

  m_brushRadius->setValue(m_settings->brushRadius());
  m_erodeRadius->setValue(m_settings->erodeRadius());
  m_dilateRadius->setValue(m_settings->dilateRadius());
  m_openRadius->setValue(m_settings->openRadius());
  m_closeRadius->setValue(m_settings->closeRadius());
}

//------------------------------------------------------------------------
void EditorToolBar::SettingsPanel::acceptChanges()
{
  m_settings->setBrushRadius (m_brushRadius->value() );
  m_settings->setErodeRadius (m_erodeRadius->value() );
  m_settings->setDilateRadius(m_dilateRadius->value());
  m_settings->setOpenRadius  (m_openRadius->value()  );
  m_settings->setCloseRadius (m_closeRadius->value() );
}

//------------------------------------------------------------------------
bool EditorToolBar::SettingsPanel::modified() const
{
  return m_brushRadius->value()  != m_settings->brushRadius()
      || m_erodeRadius->value()  != m_settings->erodeRadius()
      || m_dilateRadius->value() != m_settings->dilateRadius()
      || m_openRadius->value()   != m_settings->openRadius()
      || m_closeRadius->value()  != m_settings->closeRadius();
}

//------------------------------------------------------------------------
ISettingsPanel* EditorToolBar::SettingsPanel::clone()
{
  return new SettingsPanel(m_settings);
}



