/*

 Copyright (C) 2017 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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

// ESPINA
#include <Dialogs/CustomFileOpenDialog/OptionsPanel.h>

using namespace ESPINA;

//--------------------------------------------------------------------
OptionsPanel::OptionsPanel(QWidget* parent, Qt::WindowFlags flags)
: QWidget(parent, flags)
{
  setupUi(this);

  // default options values.
  m_usePreviousSettings->setChecked(true);
  m_useStackStreaming->setChecked(false);
  m_useStackStreaming->setEnabled(false);
  m_toolSettings->setChecked(true);
  m_checkAnalysis->setChecked(true);

  connect(m_usePreviousSettings, SIGNAL(stateChanged(int)), this, SLOT(onSettingsUseStateChanged(int)));
}

//--------------------------------------------------------------------
bool OptionsPanel::streamingValue() const
{
  if(m_usePreviousSettings->isChecked()) return false;

  return m_useStackStreaming->isChecked();
}

//--------------------------------------------------------------------
bool ESPINA::OptionsPanel::toolSettingsValue() const
{
  return m_toolSettings->isChecked();
}

//--------------------------------------------------------------------
bool OptionsPanel::checkAnalysisValue() const
{
  return m_checkAnalysis->isChecked();
}

//--------------------------------------------------------------------
void OptionsPanel::onSettingsUseStateChanged(int state)
{
  m_useStackStreaming->setEnabled(state != Qt::Checked);
}
