/*

    Copyright (C) 2013 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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
#include <Support/Settings/Settings.h>
#include "AppositionSurfaceSettings.h"
#include <QSettings>
#include <QColorDialog>

using namespace ESPINA;
using namespace ESPINA::Support::Settings;

//-----------------------------------------------------------------------------
AppositionSurfaceSettings::AppositionSurfaceSettings()
{
  setupUi(this);

  ESPINA_SETTINGS(settings);
  settings.beginGroup("Apposition Surface");

  if (settings.contains("Automatic Computation For Synapses"))
    m_automaticComputation = settings.value("Automatic Computation For Synapses").toBool();
  else
  {
    m_automaticComputation = false;
    settings.setValue("Automatic Computation For Synapses", m_automaticComputation);
  }
  settings.sync();

  m_modified = false;
  defaultComputation->setChecked(m_automaticComputation);
  connect(defaultComputation, SIGNAL(stateChanged(int)),
          this, SLOT(changeDefaultComputation(int)));
}

//-----------------------------------------------------------------------------
void AppositionSurfaceSettings::changeDefaultComputation(int value)
{
  m_automaticComputation = (Qt::Checked == value ? true : false);
  m_modified = true;
}

//-----------------------------------------------------------------------------
void AppositionSurfaceSettings::acceptChanges()
{
  if (!m_modified)
    return;

  ESPINA_SETTINGS(settings);
  settings.beginGroup("Apposition Surface");
  settings.setValue("Automatic Computation For Synapses", m_automaticComputation);
  settings.sync();
}

//-----------------------------------------------------------------------------
void AppositionSurfaceSettings::rejectChanges()
{
}

//-----------------------------------------------------------------------------
bool AppositionSurfaceSettings::modified() const
{
  return m_modified;
}

//-----------------------------------------------------------------------------
SettingsPanelPtr AppositionSurfaceSettings::clone()
{
  return new AppositionSurfaceSettings();
}