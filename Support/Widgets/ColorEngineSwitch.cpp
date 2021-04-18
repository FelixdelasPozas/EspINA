/*
 *    Copyright (C) 2015  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 *    This file is part of ESPINA.
 *
 *    ESPINA is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

// ESPINA
#include "ColorEngineSwitch.h"
#include <GUI/ColorEngines/ColorEngine.h>

using namespace ESPINA;
using namespace ESPINA::GUI::ColorEngines;
using namespace ESPINA::Support;
using namespace ESPINA::Support::Widgets;

//------------------------------------------------------------------------
ColorEngineSwitch::ColorEngineSwitch(ColorEngineSPtr engine, const QString &icon, Context &context)
: ColorEngineSwitch{engine, QIcon(icon), context}
{
}

//------------------------------------------------------------------------
ColorEngineSwitch::ColorEngineSwitch(ColorEngineSPtr engine, const QIcon &icon, Context &context)
: ProgressTool(engine->id(), icon, engine->tooltip(), context)
, m_engine(engine)
{
  setOrder("0", "1-ColorBy");

  setCheckable(true);

  setChecked(engine->isActive());

  connect(this,         SIGNAL(toggled(bool)),
          engine.get(), SLOT(setActive(bool)));
}

//------------------------------------------------------------------------
void ColorEngineSwitch::restoreSettings(std::shared_ptr<QSettings> settings)
{
  restoreCheckedState(settings);
}

//------------------------------------------------------------------------
void ColorEngineSwitch::saveSettings(std::shared_ptr< QSettings > settings)
{
  saveCheckedState(settings);
}

//------------------------------------------------------------------------
ColorEngineSPtr ColorEngineSwitch::colorEngine() const
{
  return m_engine;
}
