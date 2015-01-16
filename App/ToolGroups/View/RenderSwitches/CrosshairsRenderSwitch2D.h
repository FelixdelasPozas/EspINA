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

#ifndef ESPINA_CROSSHAIR_RENDER_SWITCH_H
#define ESPINA_CROSSHAIR_RENDER_SWITCH_H

#include <Support/RenderSwitch.h>
#include <Support/ViewManager.h>

namespace ESPINA
{
  class CrosshairsRenderSwitch2D
  : public RenderSwitch
  {
    Q_OBJECT
  public:
    explicit CrosshairsRenderSwitch2D(ViewManagerSPtr viewManager);

    virtual QWidget *widget();

    virtual ViewTypeFlags supportedViews();

  private slots:
    void setCrosshairsVisibility(bool visibile);

  private:
    ViewManagerSPtr m_viewManager;
    bool            m_crosshairsVisible;
  };
}

#endif // ESPINA_CROSSHAIR_RENDER_SWITCH_H
