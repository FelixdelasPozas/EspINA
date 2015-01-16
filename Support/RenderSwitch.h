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

#ifndef ESPINA_RENDER_SWITCH_H
#define ESPINA_RENDER_SWITCH_H

#include <GUI/View/ViewTypeFlags.h>
#include <memory>
#include <QWidget>

namespace ESPINA
{
  class RenderSwitch
  : public QObject
  {
  public:
    virtual ~RenderSwitch() {}

    void setSettingsVisibility(bool visible);

    bool settingsVisible() const
    { return m_settingsVisibility; }

    virtual QWidget *widget() = 0;

    virtual ViewTypeFlags supportedViews() = 0;

  private:
    virtual void onSettingsVisibilityChanged(bool visible) {}

  private:
    bool m_settingsVisibility;
  };

  using RenderSwitchSPtr = std::shared_ptr<RenderSwitch>;
}

#endif // ESPINA_RENDERSWITCH_H
