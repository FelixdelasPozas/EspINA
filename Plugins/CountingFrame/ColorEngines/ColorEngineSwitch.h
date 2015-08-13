/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2015  Jorge Peña Pastor <jpena@cesvima.upm.es>
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

#ifndef ESPINA_CF_COLOR_ENGINESWITCH_H
#define ESPINA_CF_COLOR_ENGINESWITCH_H

#include <Support/Widgets/ColorEngineSwitch.h>

#include "ColorEngine.h"

namespace ESPINA {
  namespace CF {

    class ColorEngineSwitch
    : public Support::Widgets::ColorEngineSwitch
    {
      Q_OBJECT
    public:
      explicit ColorEngineSwitch(CountingFrameColorEngineSPtr engine, Support::Context& context);

    private:
      void initWidgets();

    private slots:
      void onOpacityChanged(int value);

    private:
      CountingFrameColorEngineSPtr m_engine;
    };
  }
}

#endif // ESPINA_CF_COLOR_ENGINESWITCH_H
