/*
 *
 * Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 * This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
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
 */

#include <Support/Settings/Settings.h>
#include "ColorEngineMenu.h"
#include <QActionGroup>
#include <QSettings>

using namespace ESPINA;
using namespace ESPINA::GUI::ColorEngines;

const QString COLOR_ENGINE("ColorEngine");

//-----------------------------------------------------------------------------
ColorEngineMenu::ColorEngineMenu(const QString        &title,
                                 MultiColorEngineSPtr colorEngine)
: QMenu   {title}
, m_engine{colorEngine}
{
  restoreUserSettings();

  connect(this, SIGNAL(triggered(QAction*)),
          this, SLOT(toggleColorEngine(QAction*)));
}

//-----------------------------------------------------------------------------
ColorEngineMenu::~ColorEngineMenu()
{
}

//-----------------------------------------------------------------------------
void ColorEngineMenu::addColorEngine(const QString& title, ColorEngineSPtr engine)
{
  auto action = addAction(title);

  action->setCheckable(true);

  m_availableEngines[action] = engine;
}

//-----------------------------------------------------------------------------
void ColorEngineMenu::restoreUserSettings()
{
  ESPINA_SETTINGS(settings);

  QStringList activeActions = settings.value(COLOR_ENGINE, QStringList("Category")).toStringList();

  bool validColorEngine = false;

  for(auto action : m_availableEngines.keys())
  {
    if (activeActions.contains(action->text()))
    {
      action->setChecked(true);

      toggleColorEngine(action);

      validColorEngine = true;
    }
  }

  if (!validColorEngine && !m_availableEngines.isEmpty())
  {
    auto action = m_availableEngines.keys().first();

    toggleColorEngine(action);

    action->setChecked(true);
  }
}

//-----------------------------------------------------------------------------
void ColorEngineMenu::toggleColorEngine(QAction* action)
{
  if (action->isChecked())
  {
    m_engine->add(m_availableEngines[action]);
  }
  else
  {
    m_engine->remove(m_availableEngines[action]);
  }

  // Save user preferences
  ESPINA_SETTINGS(settings);

  QStringList activeActions;
  for(auto action: m_availableEngines.keys())
  {
    if (action->isChecked())
    {
      activeActions << action->text();
    }
  }

  settings.setValue(COLOR_ENGINE, activeActions);
  settings.sync();
}
