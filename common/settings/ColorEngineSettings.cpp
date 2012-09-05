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

#include "ColorEngineSettings.h"
#include <QMenu>
#include <QActionGroup>

#include <gui/TaxonomyColorEngine.h>
#include <gui/UserColorEngine.h>
#include <NumberColorEngine.h>

const QString COLOR_ENGINE("ColorEngine");

//-----------------------------------------------------------------------------
ColorEngineSettings::ColorEngineSettings()
{
  m_actions = new QActionGroup(this);

  QAction *taxonomy = new QAction(tr("Taxonomy"), this);
  taxonomy->setCheckable(true);
  m_availableEngines[taxonomy] = TaxonomyColorEngine::instance();

  QAction *user = new QAction(tr("User"), this);
  user->setCheckable(true);
  m_availableEngines[user] = new UserColorEngine();

  QAction *id = new QAction(tr("Segmentation Id"), this);
  id->setCheckable(true);
  m_availableEngines[id] = new NumberColorEngine();

  foreach(QAction *engine, m_availableEngines.keys())
  {
    m_actions->addAction(engine);
  }
  connect(m_actions, SIGNAL(triggered(QAction*)), this, SLOT(setColorEngine(QAction*)));

  taxonomy->setChecked(true);
  setColorEngine(m_availableEngines[taxonomy]);
}

//-----------------------------------------------------------------------------
QMenu *ColorEngineSettings::availableEngines()
{
  QMenu *menu = new QMenu(tr("Color By"));
  menu->addActions(m_actions->actions());
  return menu;
}

//-----------------------------------------------------------------------------
void ColorEngineSettings::setColorEngine(ColorEngine* engine)
{
  m_engine = engine;
  emit colorEngineChanged(m_engine);
}

//-----------------------------------------------------------------------------
void ColorEngineSettings::setColorEngine(QAction* engine)
{
  m_engine = m_availableEngines[engine];
  emit colorEngineChanged(m_engine);
}

