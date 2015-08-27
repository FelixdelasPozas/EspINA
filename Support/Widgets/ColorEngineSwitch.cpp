/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2015  <copyright holder> <email>
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
 *
 */

#include "ColorEngineSwitch.h"

using namespace ESPINA;
using namespace ESPINA::GUI::ColorEngines;
using namespace ESPINA::Support;
using namespace ESPINA::Support::Widgets;

//------------------------------------------------------------------------
ColorEngineSwitch::ColorEngineSwitch(ColorEngineSPtr engine, const QString &icon, Context &context)
: ColorEngineSwitch(engine, QIcon(icon), context)
{
}

//------------------------------------------------------------------------
ColorEngineSwitch::ColorEngineSwitch(ColorEngineSPtr engine, const QIcon &icon, Context &context)
: ProgressTool(engine->id(), icon, tr("Color by %1").arg(engine->tooltip()), context)
, m_engine(engine)
{
  setOredering("0", "1-ColorBy");

  setCheckable(true);

  setChecked(engine->isActive());

  connect(this,         SIGNAL(toggled(bool)),
          engine.get(), SLOT(setActive(bool)));
}

//------------------------------------------------------------------------
void ColorEngineSwitch::restoreSettings(std::shared_ptr<QSettings> settings)
{
  auto checked = checkSetting(settings);

  setChecked(checked);
}

//------------------------------------------------------------------------
void ColorEngineSwitch::saveSettings(std::shared_ptr< QSettings > settings)
{
  saveCheckSetting(settings);
}

//------------------------------------------------------------------------
ColorEngineSPtr ColorEngineSwitch::colorEngine() const
{
  return m_engine;
}
