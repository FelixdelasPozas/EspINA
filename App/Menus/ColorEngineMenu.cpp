/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2012  Jorge Peña Pastor <jpena@cesvima.upm.es>
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
 */

#include "ColorEngineMenu.h"
#include <Core/EspinaSettings.h>
#include <GUI/ViewManager.h>
#include <QMenu>
#include <QActionGroup>
#include <QSettings>

const QString COLOR_ENGINE("ColorEngine");

//-----------------------------------------------------------------------------
ColorEngineMenu::ColorEngineMenu(ViewManager *vm,
                                 const QString& title,
                                 QWidget* parent)
: QMenu(title, parent)
, m_viewManager(vm)
, m_engine(new MultiColorEngine())
{
  connect(this, SIGNAL(triggered(QAction*)),
          this, SLOT(setColorEngine(QAction*)));
}

//-----------------------------------------------------------------------------
ColorEngineMenu::~ColorEngineMenu()
{
}

//-----------------------------------------------------------------------------
void ColorEngineMenu::addColorEngine(const QString& title, ColorEnginePtr engine)
{
  QAction *taxonomy = addAction(title);
  taxonomy->setCheckable(true);
  m_availableEngines[taxonomy] = engine;
}

//-----------------------------------------------------------------------------
void ColorEngineMenu::restoreUserSettings()
{
  QSettings settings(CESVIMA, ESPINA);

  QStringList activeActions = settings.value(COLOR_ENGINE, QStringList("Taxonomy")).toStringList();

  foreach(QAction *action, m_availableEngines.keys())
  {
    if (activeActions.contains(action->text()))
    {
      action->setChecked(true);
      setColorEngine(action);
    }
  }
}

//-----------------------------------------------------------------------------
void ColorEngineMenu::setColorEngine(QAction* action)
{
  if (action->isChecked())
    m_engine->addColorEngine(m_availableEngines[action]);
  else
    m_engine->removeColorEngine(m_availableEngines[action]);

  m_viewManager->setColorEngine(m_engine.data());

  // Save user preferences
  QSettings settings(CESVIMA, ESPINA);

  QStringList activeActions;
  foreach(QAction *action, m_availableEngines.keys())
  {
    if (action->isChecked())
      activeActions << action->text();
  }

  settings.setValue(COLOR_ENGINE, activeActions);
  settings.sync();
}

